#include "TftpClient.h"

TftpClient::TftpClient(TftpClientArgs args)
    : args_(args), udp_client_(UdpClient()) {
  SetupUdpClient();

  // get the server address
  struct hostent *server;
  if ((server = gethostbyname(args.hostname.c_str())) == NULL) {
    throw UdpException("Error: Failed to get server address");
  }

  // initialize the server address structure
  memset(&server_address_, 0, sizeof(server_address_));

  server_address_.sin_family = AF_INET;  // IPv4
  // copy the server address to the server_address structure
  memcpy((char *)&server_address_.sin_addr.s_addr, (char *)server->h_addr,
         server->h_length);
  // set the port number
  server_address_.sin_port = htons(args.port);
}

void TftpClient::run() {
  Logger::Log("TFTP client started\n");
  try {
    switch (args_.mode) {
      case TftpClientArgs::TftpMode::READ:
        Read();
        break;
      case TftpClientArgs::TftpMode::WRITE:
        Write();
        break;
    }
  } catch (TFTPFileNotFoundError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::FILE_NOT_FOUND, e.what());
    udp_client_.Send(error.MakeRaw(), server_address_);
    throw;
  } catch (TFTPAccessViolationError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::ACCESS_VIOLATION, e.what());
    udp_client_.Send(error.MakeRaw(), server_address_);
    throw;
  } catch (TFTPDiskFullError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::DISK_FULL, e.what());
    udp_client_.Send(error.MakeRaw(), server_address_);
    throw;
  } catch (TFTPIllegalOperationError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION, e.what());
    udp_client_.Send(error.MakeRaw(), server_address_);
    throw;
  } catch (TFTPUnknownTransferIDError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::UNKNOWN_TID, e.what());
    udp_client_.Send(error.MakeRaw(), server_address_);
    throw;
  } catch (TFTPFileAlreadyExistsError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::FILE_ALREADY_EXISTS, e.what());
    udp_client_.Send(error.MakeRaw(), server_address_);
    throw;
  } catch (TFTPNoSuchUserError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::NO_SUCH_USER, e.what());
    udp_client_.Send(error.MakeRaw(), server_address_);
    throw;
  } catch (TTFOptionNegotiationError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::FAILED_NEGOTIATION, e.what());
    udp_client_.Send(error.MakeRaw(), server_address_);
    throw;
  }
}

void TftpClient::SetupUdpClient() {
  Option *blksize_ext = NULL;
  Option *timeout_ext = NULL;
  for (auto &option : args_.options) {
    switch (option.name) {
      case Option::Name::BLKSIZE:
        blksize_ext = &option;
        break;
      case Option::Name::TIMEOUT:
        timeout_ext = &option;
        break;
      default:
        break;
    }
  }
  if (blksize_ext && timeout_ext) {
    udp_client_.ChangeTimeout({stoi(timeout_ext->value), 0});
    udp_client_.ChangeMaxPacketSize(stoi(blksize_ext->value));

  } else if (blksize_ext) {
    udp_client_.ChangeTimeout({0, 0});
    udp_client_.ChangeMaxPacketSize(stoi(blksize_ext->value));
  } else if (timeout_ext) {
    udp_client_.ChangeTimeout({stoi(timeout_ext->value), 0});
    udp_client_.ChangeMaxPacketSize(512);
  }
}

std::vector<uint8_t> TftpClient::RecievePacketFromServer() {
  std::vector<uint8_t> buffer;

  // Create a unique_ptr to a sockaddr_in object to store the sender's address
  std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);

  // Receive data from any sender and capture the sender's address
  buffer = udp_client_.Receive(sender_address.get());

  int retries = 0;
  while (sender_address->sin_port != server_address_.sin_port &&
         retries < numRetries) {
    // Keep receiving data until the sender's port matches the server's port or
    // the number of retries exceeds the maximum number of retries

    // respond with error packet to the unknown sender
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::UNKNOWN_TID, "Unknown sender");
    try {
      udp_client_.Send(error.MakeRaw(), *sender_address.get());
    } catch (UdpException &e) {
      Logger::Log("Error: Failed to send error packet to unknown sender\n");
    }

    buffer = udp_client_.Receive(sender_address.get());
    retries++;
  }

  if (retries >= numRetries) {
    throw UdpTimeoutException();  // TODO
  }

  return buffer;
}

void TftpClient::ValidateOptionsInOack(std::vector<Option> oack_options) {
  // validate that all options in OACK packet also exist in client options and
  // if they do:
  // - the option 'blksize' in OACK packet has the same or lower value than
  // the option 'blksize' in client,
  // - the option 'timeout' in OACK packet has the same or lower value than
  // the option 'timeout' in client,
  // - the option 'tsize' in OACK packet has the same value as the option
  // 'tsize' in client
  for (auto oack_option : oack_options) {
    bool found = false;
    for (auto client_option : args_.options) {
      if (oack_option.name == client_option.name) {
        found = true;
        if (oack_option.name == Option::Name::BLKSIZE) {
          if (stoi(oack_option.value) > stoi(client_option.value)) {
            throw TFTPIllegalOperationError(
                "Server responded with option 'blksize' with value higher "
                "than client requested");
          }
        }
        if (oack_option.name == Option::Name::TIMEOUT) {
          if (stoi(oack_option.value) > stoi(client_option.value)) {
            throw TFTPIllegalOperationError(
                "Server responded with option 'timeout' with value higher "
                "than client requested");
          }
        }
        if (oack_option.name == Option::Name::TSIZE) {
          if (stoi(oack_option.value) != stoi(client_option.value)) {
            throw TFTPIllegalOperationError(
                "Server responded with option 'tsize' with value different "
                "than client requested");
          }
        }
        if (oack_option.name == Option::Name::UNSUPPORTED) {
          throw TFTPIllegalOperationError(
              "Server responded with unsupported option");
        }
      }
    }
    if (!found) {
      throw TFTPIllegalOperationError("Server responded with option '" +
                                      oack_option.nameStr +
                                      "' that client did not request");
    }
  }
}
