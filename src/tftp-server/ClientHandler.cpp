#include "ClientHandler.h"

ClientHandler::ClientHandler(std::string hostname, int port,
                             std::string root_dirpath)
    : udp_client_(UdpClient()), root_dirpath_(root_dirpath) {
  // initialize the client address structure
  memset(&client_address_, 0, sizeof(client_address_));

  client_address_.sin_family = AF_INET;  // IPv4
  // Set the IPv4 address directly
  inet_pton(AF_INET, hostname.c_str(), &client_address_.sin_addr);
  // set the port number
  client_address_.sin_port = htons(port);
}

void ClientHandler::FollowOnIntroPacket(std::vector<uint8_t> intro_packet) {
  // Parse the intro packet
  TftpPacket::Opcode opcode_1;
  try {  // try to get the opcode
    opcode_1 = TftpPacket::GetOpcodeFromRaw(intro_packet);
  } catch (TFTPIllegalOperationError &e) {
    Logger::Log("Invalid opcode in intro packet\n");
    auto error = ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                             "Invalid opcode in intro packet");
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  }
  try {  // try to follow on the intro packet and handle TFTP errors
    if (opcode_1 == TftpPacket::Opcode::RRQ) {
      FollowOnRRQ(intro_packet);
    } else if (opcode_1 == TftpPacket::Opcode::WRQ) {
      FollowOnWRQ(intro_packet);
    }
  } catch (TFTPFileNotFoundError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::FILE_NOT_FOUND, e.what());
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  } catch (TFTPAccessViolationError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::ACCESS_VIOLATION, e.what());
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  } catch (TFTPDiskFullError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::DISK_FULL, e.what());
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  } catch (TFTPIllegalOperationError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION, e.what());
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  } catch (TFTPUnknownTransferIDError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::UNKNOWN_TID, e.what());
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  } catch (TFTPFileAlreadyExistsError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::FILE_ALREADY_EXISTS, e.what());
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  } catch (TFTPNoSuchUserError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::NO_SUCH_USER, e.what());
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  } catch (TTFOptionNegotiationError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::FAILED_NEGOTIATION, e.what());
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  }
}

void ClientHandler::FollowOnRRQ(std::vector<uint8_t> intro_packet) {
  auto rrq = ReadWritePacket(intro_packet);
  Logger::Log("received RRQ packet");
  auto read_file = root_dirpath_ + "/" + rrq.filepath;
  Reader reader(root_dirpath_ + "/" + rrq.filepath,
                rrq.mode == ReadWritePacket::Mode::OCTET
                    ? DataFormat::OCTET
                    : DataFormat::NETASCII);
  // check if file exists
  if (!FileHandler::FileExists(read_file)) {
    throw TFTPFileNotFoundError("File does not exist: " + rrq.filepath);
  }
  // update io handler to use the requested
  // negotiate options
  auto negotiated_options = NegotiateOptions(rrq.options);

  // if options were negotiated, send oack and get ack
  if (rrq.options.size() > 0) {
    // send oack
    auto oack = OackPacket(negotiated_options);
    Logger::Log("sending OACK packet");
    udp_client_.Send(oack.MakeRaw(), client_address_);
    // get ack
    int retries = 0;
    while (retries < numRetries) {
      try {
        auto packet = RecievePacketFromClient();
        if (TftpPacket::GetOpcodeFromRaw(packet) == TftpPacket::Opcode::ACK) {
          auto ack = AckPacket(packet);
          if (ack.block_number != 0) {
            throw TFTPIllegalOperationError("Expected block number 0");
          }
          Logger::Log("received ACK packet");
          break;
        } else if (TftpPacket::GetOpcodeFromRaw(packet) ==
                   TftpPacket::Opcode::RRQ) {
          retries++;
        } else if (TftpPacket::GetOpcodeFromRaw(packet) ==
                   TftpPacket::Opcode::ERROR) {
          auto error = ErrorPacket(packet);
          throw RecievedErrorPacketException("Client responded with error: " +
                                             error.error_message);
        } else {
          throw TFTPIllegalOperationError("Expected ACK or ERROR");
        }
      } catch (UdpTimeoutException &e) {
        retries++;
      }
    }
    if (retries >= numRetries) {
      throw TFTPIllegalOperationError("Error: Timeout");
    }
  }

  int block_number = 1;
  uint16_t blksize = 512;
  // if 'blksize' option is specified, set blksize to the value of 'blksize'
  // option
  for (auto option : negotiated_options) {
    if (option.name == Option::Name::BLKSIZE) {
      blksize = stoi(option.value);
    }
  }
  bool last_packet_sent = false;
  // keep sending data packets until the file is completely sent
  while (!last_packet_sent) {
    auto data = reader.ReadFile(blksize);  // read data from file
    if (data.size() < blksize) {
      // last packet, set flag to true
      last_packet_sent = true;
    }
    // keep sending DATA packets until an ACK packet with the correct block
    // number is received or retry limit is reached
    int retries = 0;
    bool ackReceived = false;
    while (!ackReceived && retries < numRetries) {
      try {
        // create and send DATA packet
        auto data_packet = DataPacket(block_number, data);
        Logger::Log("sending DATA packet");
        udp_client_.Send(data_packet.MakeRaw(), client_address_);
        auto response_n = RecievePacketFromClient();
        auto opcode_n = TftpPacket::GetOpcodeFromRaw(response_n);
        if (opcode_n == TftpPacket::Opcode::ACK) {  // ACK received
          auto ack_n = AckPacket(response_n);
          if (ack_n.block_number == block_number) {
            // ACK received for the correct block number, increment block
            // number
            ackReceived = true;
            block_number++;
          } else if (ack_n.block_number <=
                     block_number) {  // ACK received for an old block
                                      // number, try again
            retries++;
          } else {  // ACK received for a future block number, throw error
            throw TFTPIllegalOperationError(
                "Expected ACK packet with block number " +
                std::to_string(block_number) + ", got block number " +
                std::to_string(ack_n.block_number));
          }
        } else if (opcode_n == TftpPacket::Opcode::ERROR) {  // ERROR received
          auto error = ErrorPacket(response_n);
          throw RecievedErrorPacketException("Client responded with error: " +
                                             error.error_message);
        }
      } catch (UdpTimeoutException &e) {  // timeout
        retries++;
        if (retries >= numRetries) {
          if (!last_packet_sent)  // timed out before sending last packet
            throw TFTPIllegalOperationError("Error: Timeout");
          else  // sent last packet but did not recieve ACK, assume client
                // recieved last packet and terminate
            break;
        }
      }
    }
    if (retries >= numRetries) {
      if (retries >= numRetries) {
        if (!last_packet_sent)  // timed out before sending last packet
          throw TFTPIllegalOperationError("Error: Timeout");
        else  // sent last packet but did not recieve ACK, assume client
              // recieved last packet and terminate
          break;
      }
    }
  }
}

void ClientHandler::FollowOnWRQ(std::vector<uint8_t> intro_packet) {}

std::vector<Option> ClientHandler::NegotiateOptions(
    std::vector<Option> options) {
  std::vector<Option> negotiated_options;
  for (auto option : options) {
    switch (option.name) {
      case Option::Name::BLKSIZE:
        udp_client_.ChangeMaxPacketSize(stoi(option.value));
        negotiated_options.push_back(option);
        break;
      case Option::Name::TIMEOUT:
        udp_client_.ChangeTimeout({stoi(option.value), 0});
        negotiated_options.push_back(option);
        break;
      case Option::Name::TSIZE:
        // TODO: implement
        negotiated_options.push_back(option);
        break;
      case Option::Name::UNSUPPORTED:  // ignore
        break;
    }
  }
  return negotiated_options;
}

std::vector<uint8_t> ClientHandler::RecievePacketFromClient() {
  std::vector<uint8_t> buffer;

  // Create a unique_ptr to a sockaddr_in object to store the sender's address
  std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);

  // Receive data from any sender and capture the sender's address
  buffer = udp_client_.Receive(sender_address.get());

  int retries = 0;
  while (sender_address->sin_port != client_address_.sin_port &&
         retries < numRetries) {
    // Keep receiving data until the sender's port matches the client's port or
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