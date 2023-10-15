#include "TftpClient.h"

TftpClient::TftpClient(TftpClientArgs args)
    : args_(args), udp_client_(UdpClient(args_.hostname, args_.port, {}, {})) {
  SetupUdpClient();
}

void TftpClient::run() {
  printf("TFTP client started\n");
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
  } catch (TFTPIllegalOperationError &e) {  // incomming packet is not a correct
                                            // TFTP packet
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION, e.what());
    udp_client_.Send(error.MakeRaw());
    throw TftpClientException(e.what());
  }
  // check if opcode is ACK, OACK, or ERROR
  if (opcode_1 == TftpPacket::Opcode::ACK) {  // server responded with ACK
    auto ack = AckPacket(response_1);
    if (ack.block_number != 0) {
      auto error_end = ErrorPacket(
          ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
          "Expected ACK packet with block number 0, got block number " +
              std::to_string(ack.block_number));
      udp_client_.Send(error_end.MakeRaw());
      throw TftpClientException(
          "Expected ACK packet with block number 0, got "
          "block number " +
          std::to_string(ack.block_number));
    }
    // if client tried to send options, but server responded with ACK, then
    // server does not support options and client will move on without options
    args_.options = {};
    // set up udp_client_ to use default block size and timeout (keep port)
    SetupUdpClient();
  } else if (opcode_1 == TftpPacket::Opcode::OACK) {  // server responded with
                                                      // OACK
    auto oack = OackPacket(response_1);
    // validate OACK packet
    if (args_.options.size() < oack.options.size()) {
      auto error = ErrorPacket(
          ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
          "Server responded with more options than client requested");
      udp_client_.Send(error.MakeRaw());
      throw TftpClientException(
          "Server responded with more options than client requested");
    }
    ValidateOptionsInOack(oack.options);
    // use options from OACK packet
    // set up udp_client_ to use options from OACK packet (keep port)
    args_.options = oack.options;
    SetupUdpClient();
  } else if (opcode_1 == TftpPacket::Opcode::ERROR) {  // server responded with
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
      if (opcode_n == TftpPacket::Opcode::ACK) {  // ACK received
        auto ack_n = AckPacket(response_n);
        if (ack_n.block_number == block_number) {
          // ACK received for the correct block number, increment block number
          block_number++;
          ack_received = true;
        }
      } else if (opcode_n == TftpPacket::Opcode::ERROR) {  // ERROR received
        file.close();
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

void TftpClient::Read() {
  // send RRQ packet
  auto rrq = ReadWritePacket(TftpPacket::Opcode::RRQ, args_.filepath,
                             ReadWritePacket::Mode::OCTET, args_.options);
  udp_client_.Send(rrq.MakeRaw());
  // receive DATA / OACK / ERROR packet
  std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);
  auto response_1 = udp_client_.ReceiveFromAny(sender_address.get());
  // change port to the one that the first server response came from
  udp_client_.ChangePort(sender_address->sin_port);
  TftpPacket::Opcode opcode_1;
  // check if incomming packet is a correct TFTP packet
  try {
    opcode_1 = TftpPacket::GetOpcodeFromRaw(response_1);
  } catch (TFTPIllegalOperationError &e) {  // incomming packet is not a correct
                                            // TFTP packet
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION, e.what());
    udp_client_.Send(error.MakeRaw());
    throw TftpClientException(e.what());
  }
  // declare block number
  // - if client sends RRQ with options, but server responds with DATA, then
  // server does not support options and client will move on without options
  // send ACK packet with block number 1
  // - if client sends RRQ with options, and server responds with OACK, then
  // client will respond with ACK packet with block number 0
  uint16_t block_number;
  // open file for writing
  std::ofstream file(args_.dest_filepath, std::ios::binary);
  if (!file) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::FILE_NOT_FOUND, "File not found");
    udp_client_.Send(error.MakeRaw());
    throw TftpClientException("File not found");
  }
  // check if opcode is DATA, OACK, or ERROR
  if (opcode_1 == TftpPacket::Opcode::DATA) {  // server responded with DATA
    auto data = DataPacket(response_1);
    if (data.block_number != 1) {
      auto error = ErrorPacket(
          ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
          "Expected DATA packet with block number 1, got block number " +
              std::to_string(data.block_number));
      udp_client_.Send(error.MakeRaw());
      throw TftpClientException(
          "Expected DATA packet with block number 1, got "
          "block number " +
          std::to_string(data.block_number));
    }
    // that means that server does not support options and client will move on
    // without options
    args_.options = {};
    // set up udp_client_ to use default block size and timeout (keep port)
    SetupUdpClient();
    // set block number to 1
    block_number = 1;
    // write data to file
    file.write((char *)data.data.data(), data.data.size());
  } else if (opcode_1 == TftpPacket::Opcode::OACK) {  // server responded with
                                                      // OACK
    auto oack = OackPacket(response_1);
    // validate OACK packet
    if (args_.options.size() < oack.options.size()) {
      auto error = ErrorPacket(
          ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
          "Server responded with more options than client requested");
      udp_client_.Send(error.MakeRaw());
      throw TftpClientException(
          "Server responded with more options than client requested");
    }
    ValidateOptionsInOack(oack.options);
    // use options from OACK packet
    // set up udp_client_ to use options from OACK packet (keep port)
    args_.options = oack.options;
    SetupUdpClient();
    // set block number to 0
    block_number = 0;
  } else if (opcode_1 == TftpPacket::Opcode::ERROR) {  // server responded with
                                                       // ERROR
    auto error = ErrorPacket(response_1);
    // TODO: client could send a new RRQ packet without options to retry
    throw TftpClientException(error.error_message);
  } else {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                             "Expected DATA, OACK, or ERROR packet");
    udp_client_.Send(error.MakeRaw());
    throw TftpClientException("Expected DATA, OACK, or ERROR packet");
  }
  // send ACK packet
  auto ack = AckPacket(block_number);
  udp_client_.Send(ack.MakeRaw());
  // define blksize
  uint16_t blksize = 512;
  // if 'blksize' option is specified, set blksize to the value of 'blksize'
  // option
  for (auto option : args_.options) {
    if (option.name == Option::Name::BLKSIZE) {
      blksize = stoi(option.value);
    }
  }
  // receive DATA packets
  // a loop that will continue until the last DATA packet containing less than
  // 512 bytes (or 'blksize' option value if specified) of data is received or
  // until the client receives an ERROR packet
  bool last_packet_received = false;
  while (!last_packet_received) {
    // receive DATA packet
    auto response_n = udp_client_.ReceiveFromSpecific();
    TftpPacket::Opcode opcode_n = TftpPacket::GetOpcodeFromRaw(response_n);
    if (opcode_n == TftpPacket::Opcode::DATA) {  // DATA received
      auto data_n = DataPacket(response_n);
      if (data_n.block_number == block_number + 1) {
        // DATA received for the correct block number, increment block number
        block_number++;
        // write data to file
        file.write((char *)data_n.data.data(), data_n.data.size());
        // check if this is the last DATA packet
        if (data_n.data.size() < blksize) {
          // last packet, set flag to true
          last_packet_received = true;
        }
        // send ACK packet
        auto ack_n = AckPacket(block_number);
        udp_client_.Send(ack_n.MakeRaw());
      }
    } else if (opcode_n == TftpPacket::Opcode::ERROR) {  // ERROR received
      file.close();
      auto error = ErrorPacket(response_n);
      throw TftpClientException(error.error_message);
    }
    // try again
  }
  // close file
  file.close();

  // gratz, file received, all good
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
  udp_client_ = UdpClient(args_.hostname, args_.port, {}, {});
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
