#include "TftpClient.h"

TftpClient::TftpClient(TftpClientArgs args)
    : io_handler(IOHandler(args.mode == TftpClientArgs::TftpMode::READ
                               ? IOHandler::IO::FILE
                               : IOHandler::IO::STD)),
      args_(args),
      udp_client_(UdpClient(args_.hostname, args_.port)) {
  if (io_handler.io_type == IOHandler::IO::FILE) {
    io_handler.OpenFile(args_.dest_filepath, std::ios_base::binary);
  }
  SetupUdpClient();
}

void TftpClient::run() {
  Logger::Log("TFTP client started\n");
  switch (args_.mode) {
    case TftpClientArgs::TftpMode::READ:
      Read();
      break;
    case TftpClientArgs::TftpMode::WRITE:
      Write();
      break;
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
            auto error = ErrorPacket(
                ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                "Server responded with option 'blksize' with value higher "
                "than client requested");
            udp_client_.Send(error.MakeRaw());
            throw TftpClientException(
                "Server responded with option 'blksize' with value higher "
                "than client requested");
          }
        }
        if (oack_option.name == Option::Name::TIMEOUT) {
          if (stoi(oack_option.value) > stoi(client_option.value)) {
            auto error = ErrorPacket(
                ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                "Server responded with option 'timeout' with value higher "
                "than client requested");
            udp_client_.Send(error.MakeRaw());
            throw TftpClientException(
                "Server responded with option 'timeout' with value higher "
                "than client requested");
          }
        }
        if (oack_option.name == Option::Name::TSIZE) {
          if (stoi(oack_option.value) != stoi(client_option.value)) {
            auto error = ErrorPacket(
                ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                "Server responded with option 'tsize' with value different "
                "than client requested");
            udp_client_.Send(error.MakeRaw());
            throw TftpClientException(
                "Server responded with option 'tsize' with value different "
                "than client requested");
          }
        }
      }
    }
    if (!found) {
      auto error =
          ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                      "Server responded with option '" + oack_option.nameStr +
                          "' that client did not request");
      udp_client_.Send(error.MakeRaw());
      throw TftpClientException("Server responded with option '" +
                                oack_option.nameStr +
                                "' that client did not request");
    }
  }
}
