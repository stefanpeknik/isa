#include "TftpClient.h"

TftpClient::TftpClient(TftpClientArgs args)
    : args_(args), udp_client_(UdpClient(args_.hostname, args_.port, {}, {})) {
  Option *blksize_ext = NULL;
  Option *timeout_ext = NULL;
  for (auto option : args_.options) {
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
    udp_client_ =
        UdpClient(args_.hostname, args_.port, stoi(blksize_ext->value),
                  {stoi(timeout_ext->value), 0});
  } else if (blksize_ext) {
    udp_client_ =
        UdpClient(args_.hostname, args_.port, stoi(blksize_ext->value), {});
  } else if (timeout_ext) {
    udp_client_ = UdpClient(args_.hostname, args_.port, {},
                            {stoi(timeout_ext->value), 0});
  }
}

void TftpClient::run() {
  switch (args_.mode) {
  case TftpClientArgs::TftpMode::READ:
    Read();
    break;
  case TftpClientArgs::TftpMode::WRITE:
    Write();
    break;
  }
}

void TftpClient::Write() {

  // TODO : make the code look cleaner

  // send WRQ packet
  auto wrq = ReadWritePacket(TftpPacket::Opcode::WRQ, args_.filepath,
                             ReadWritePacket::Mode::OCTET, args_.options);
  udp_client_.Send(wrq.MakeRaw());

  // receive ACK / OACK / ERROR packet
  std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);
  auto response_1 = udp_client_.ReceiveFromAny(sender_address.get());
  // change port to the one that the first server response came from
  udp_client_.ChangePort(sender_address->sin_port);
  TftpPacket::Opcode opcode_1;
  // check if incomming packet is a correct TFTP packet
  try {
    opcode_1 = TftpPacket::GetOpcodeFromRaw(response_1);
  } catch (TFTPIllegalOperationError &e) { // incomming packet is not a correct
                                           // TFTP packet
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION, e.what());
    udp_client_.Send(error.MakeRaw());
    throw TftpClientException(e.what());
  }
  // check if opcode is ACK, OACK, or ERROR
  if (opcode_1 == TftpPacket::Opcode::ACK) { // server responded with ACK
    auto ack = AckPacket(response_1);
    if (ack.block_number != 0) {
      auto error_end = ErrorPacket(
          ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
          "Expected ACK packet with block number 0, got block number " +
              std::to_string(ack.block_number));
      udp_client_.Send(error_end.MakeRaw());
      throw TftpClientException("Expected ACK packet with block number 0, got "
                                "block number " +
                                std::to_string(ack.block_number));
    }
    // if client tried to send options, but server responded with ACK, then
    // server does not support options and client will move on without options
    args_.options = {};
    // set up udp_client_ to use default block size and timeout (keep port)
    udp_client_ =
        UdpClient(args_.hostname, htons(sender_address->sin_port), {}, {});
  } else if (opcode_1 == TftpPacket::Opcode::OACK) { // server responded with
                                                     // OACK
    auto oack = OackPacket(response_1);
    // validate options in OACK packet
    if (args_.options.size() < oack.options.size()) {
      auto error = ErrorPacket(
          ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
          "Server responded with more options than client requested");
      udp_client_.Send(error.MakeRaw());
      throw TftpClientException(
          "Server responded with more options than client requested");
    }
    // validate that all options in OACK packet also exist in client options and
    // if they do:
    // - the option 'blksize' in OACK packet has the same or lower value than
    // the option 'blksize' in client,
    // - the option 'timeout' in OACK packet has the same or lower value than
    // the option 'timeout' in client,
    // - the option 'tsize' in OACK packet has the same value as the option
    // 'tsize' in client
    for (auto oack_option : oack.options) {
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
    // use options from OACK packet
    args_.options = oack.options;
    // set up udp_client_ to use options from OACK packet (keep port)
    Option *blksize_ext = NULL;
    Option *timeout_ext = NULL;
    for (auto option : args_.options) {
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
      udp_client_ =
          UdpClient(args_.hostname, args_.port, stoi(blksize_ext->value),
                    {stoi(timeout_ext->value), 0});
    } else if (blksize_ext) {
      udp_client_ =
          UdpClient(args_.hostname, args_.port, stoi(blksize_ext->value), {});
    } else if (timeout_ext) {
      udp_client_ = UdpClient(args_.hostname, args_.port, {},
                              {stoi(timeout_ext->value), 0});
    }
  } else if (opcode_1 == TftpPacket::Opcode::ERROR) { // server responded with
                                                      // ERROR
    auto error = ErrorPacket(response_1);
    // TODO: client could send a new WRQ packet without options to retry
    throw TftpClientException(error.error_message);
  } else {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                             "Expected ACK, OACK, or ERROR packet");
    udp_client_.Send(error.MakeRaw());
    throw TftpClientException("Expected ACK, OACK, or ERROR packet");
  }

  // send DATA packets
  // a loop that will continue until the last DATA packet containing less than
  // blksize bytes (or 'blksize' option value if specified) of data is sent or
  // until the client receives an ERROR packet
  uint16_t block_number = 1;
  uint16_t blksize = 512;
  // if 'blksize' option is specified, set blksize to the value of 'blksize'
  // option
  for (auto option : args_.options) {
    if (option.name == Option::Name::BLKSIZE) {
      blksize = stoi(option.value);
    }
  }
  // open file
  std::ifstream file(args_.filepath, std::ios::binary);
  if (!file) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::FILE_NOT_FOUND, "File not found");
    udp_client_.Send(error.MakeRaw());
    throw TftpClientException("File not found");
  }
  // loop that will continue until the last DATA packet containing less than
  // blksize bytes of data is sent or until the client receives an ERROR packet
  bool last_packet_sent = false;
  while (!last_packet_sent) {
    // read a piece of data from the file
    std::vector<char> data(blksize);
    file.read(data.data(), blksize);
    std::streamsize bytes_read = file.gcount();
    if (bytes_read < blksize) {
      // last packet, set flag to true
      last_packet_sent = true;
    }
    // convert the data vector to a vector of uint8_t
    std::vector<uint8_t> data_uint8(data.begin(), data.begin() + bytes_read);
    // keep sending DATA packets until an ACK packet with the correct block
    // number is received
    bool ack_received = false;
    while (!ack_received) {
      // create and send DATA packet
      DataPacket data_packet(block_number, data_uint8);
      udp_client_.Send(data_packet.MakeRaw());
      auto response_n = udp_client_.ReceiveFromSpecific();
      TftpPacket::Opcode opcode_n = TftpPacket::GetOpcodeFromRaw(response_n);
      if (opcode_n == TftpPacket::Opcode::ACK) { // ACK received
        auto ack_n = AckPacket(response_n);
        if (ack_n.block_number == block_number) {
          // ACK received for the correct block number, increment block number
          block_number++;
          ack_received = true;
        }
      } else if (opcode_n == TftpPacket::Opcode::ERROR) { // ERROR received
        auto error = ErrorPacket(response_n);
        throw TftpClientException(error.error_message);
      }
      // try again
      // TODO : might be a good idea to implement a timeout here?
    }
  }
  // close file
  file.close();

  // gratz, file sent, all good
}

void TftpClient::Read() {}
