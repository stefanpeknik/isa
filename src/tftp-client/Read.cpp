#include "TftpClient.h"

void TftpClient::Read() {
  // setup file handler
  Writer writer(args_.dest_filepath,
                args_.file_mode == ReadWritePacket::Mode::OCTET
                    ? DataFormat::OCTET
                    : DataFormat::NETASCII);
  CheckForSigintRRQ(&writer);
  // define blksize
  uint16_t blksize = 512;
  // bool to check if last packet was received
  bool last_packet_received = false;
  // declare block number
  // - if client sends RRQ with options, but server responds with DATA, then
  // server does not support options and client will move on without options
  // send ACK packet with block number 1
  // - if client sends RRQ with options, and server responds with OACK, then
  // client will respond with ACK packet with block number 0
  uint16_t block_number;
  uint8_t retries = 0;
  while (retries < MAX_RETRIES) {
    try {
      // send RRQ packet
      auto rrq = ReadWritePacket(TftpPacket::Opcode::RRQ, args_.filepath,
                                 ReadWritePacket::Mode::OCTET, args_.options);
      Logger::Log("sending RRQ packet");
      udp_client_.Send(rrq.MakeRaw(), server_address_);
      // receive DATA / OACK / ERROR packet
      std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);
      std::vector<uint8_t> response_1;
      try {
        response_1 = udp_client_.Receive(sender_address.get());
      } catch (UdpSigintException &e) {
        CheckForSigintRRQ(&writer);
      }
      // change port to the one that the first server response came from
      server_address_.sin_port = sender_address->sin_port;
      Logger::Log("New port: " + std::to_string(server_address_.sin_port));
      LogPotentialTftpPacket(server_address_, response_1);
      // check if incomming packet is a correct TFTP packet
      auto opcode_1 = TftpPacket::GetOpcodeFromRaw(response_1);
      // check if opcode is DATA, OACK, or ERROR
      if (opcode_1 == TftpPacket::Opcode::DATA) { // server responded with DATA
        Logger::Log("received DATA packet");
        auto data = DataPacket(response_1);
        if (data.block_number != 1) {
          throw TFTPIllegalOperationError(
              "Expected DATA packet with block number 1, got "
              "block number " +
              std::to_string(data.block_number));
        }
        // that means that server does not support options and client will move
        // on without options
        args_.options = {};
        // set up udp_client_ to use default block size and timeout (keep port)
        SetupUdpClient();
        // set block number to 1
        block_number = 1;
        // if data size is less than 512 bytes, then this is the last packet
        if (data.data.size() < blksize) {
          last_packet_received = true;
        }
        // write data to file
        try {
          writer.WriteFile(data.data);
        } catch (FailedToWriteToFileException &e) {
          throw TFTPDiskFullError();
        } catch (FailedToOpenFileException &e) {
          throw TFTPAccessViolationError();
        }
        break; // break out of the loop
      } else if (opcode_1 == TftpPacket::Opcode::OACK) { // server responded
                                                         // with OACK
        Logger::Log("received OACK packet");
        auto oack = OackPacket(response_1);
        // validate OACK packet
        if (args_.options.size() < oack.options.size()) {
          throw TFTPIllegalOperationError(
              "Server responded with more options than client requested");
        }
        ValidateOptionsInOack(oack.options);
        // use options from OACK packet
        // set up udp_client_ to use options from OACK packet (keep port)
        args_.options = oack.options;
        // if tsize option is specified, check if there is enough space for file
        // to be read
        // TODO
        for (auto option : args_.options) {
          if (option.name == Option::Name::TSIZE) {
            uintmax_t tsize = std::stoull(option.value);
            if (!FileHandler::HasEnoughSpace("./", tsize)) {
              throw NotEnoughSpaceException();
            }
          }
        }
        SetupUdpClient();
        // set block number to 0
        block_number = 0;
        break; // break out of the loop
      } else if (opcode_1 == TftpPacket::Opcode::ERROR) { // server responded
                                                          // with ERROR
        Logger::Log("received ERROR packet");
        auto error = ErrorPacket(response_1);
        // NOTE: client could send a new RRQ packet without options to retry
        throw RecievedErrorPacketException("Server responded with error: " +
                                           error.error_message);
      } else {
        throw TFTPIllegalOperationError("Expected DATA, OACK, or ERROR packet");
      }
    } catch (UdpTimeoutException &e) { // timeout
      retries++;
      udp_client_.IncreaseTimeout();
    }
    CheckForSigintRRQ(&writer);
  }
  if (retries >= MAX_RETRIES) {
    throw TFTPIllegalOperationError("Error: Timeout");
  }
  CheckForSigintRRQ(&writer);

  // reset timeout
  udp_client_.TimeoutReset();

  // so far, client has sent RRQ packet and received DATA / OACK

  // if 'blksize' option is specified, set blksize to the value of 'blksize'
  // option
  for (auto option : args_.options) {
    if (option.name == Option::Name::BLKSIZE) {
      blksize = stoi(option.value);
    }
  }
  CheckForSigintRRQ(&writer);

  // send ACKs and receive DATA packets
  // a loop that will continue until the last DATA packet containing less
  // than 512 bytes (or 'blksize' option value if specified) of data is
  // received or until the client receives an ERROR packet
  while (!last_packet_received) {
    retries = 0;
    auto dataReceived = false;
    while (!dataReceived && retries < MAX_RETRIES) {
      try {
        CheckForSigintRRQ(&writer);
        // send ACK packet
        auto ack = AckPacket(block_number);
        Logger::Log("sending ACK packet for block number " +
                    std::to_string(block_number));
        udp_client_.Send(ack.MakeRaw(), server_address_);
        // receive DATA packet
        auto response_n = RecievePacketFromServer();
        TftpPacket::Opcode opcode_n = TftpPacket::GetOpcodeFromRaw(response_n);
        if (opcode_n == TftpPacket::Opcode::DATA) { // DATA received
          auto data_n = DataPacket(response_n);
          if (data_n.block_number == block_number + 1) {
            // DATA received for the correct block number, increment block
            // number
            dataReceived = true;
            block_number++;
            Logger::Log("received DATA packet for block number " +
                        std::to_string(data_n.block_number));
            // write data to file
            try {
              writer.WriteFile(data_n.data);
            } catch (FailedToWriteToFileException &e) {
              throw TFTPDiskFullError();
            } catch (FailedToOpenFileException &e) {
              throw TFTPAccessViolationError();
            }
            // check if this is the last DATA packet
            if (data_n.data.size() < blksize) {
              // last packet, set flag to true
              last_packet_received = true;
            }
          } else if (data_n.block_number < block_number + 1) {
            // DATA received for an old block number, try again
            Logger::Log("received DATA packet for an old block number");
            retries++;
            // increase timeout
            udp_client_.IncreaseTimeout();
          } else {
            throw TFTPIllegalOperationError(
                "Expected DATA packet with block number " +
                std::to_string(block_number + 1) + ", got block number " +
                std::to_string(data_n.block_number));
          }
        } else if (opcode_n == TftpPacket::Opcode::ERROR) { // ERROR received
          Logger::Log("received ERROR packet");
          auto error = ErrorPacket(response_n);
          throw RecievedErrorPacketException("Server responded with error: " +
                                             error.error_message);
        }
        // try again
      } catch (UdpTimeoutException &e) { // timeout
        retries++;
        udp_client_.IncreaseTimeout();
      } catch (UdpSigintException &e) {
        CheckForSigintRRQ(&writer);
      } // every other exception is immidiately considered fatal and is thrown
      CheckForSigintRRQ(&writer);
    }
    if (retries >= MAX_RETRIES) {
      throw TFTPIllegalOperationError("Error: Timeout");
    }
    // reset timeout
    udp_client_.TimeoutReset();
  }
  CheckForSigintRRQ(&writer);
  // as a good manner send last ACK for last DATA
  auto ack = AckPacket(block_number);
  udp_client_.Send(ack.MakeRaw(), server_address_);
  Logger::Log("sending ACK packet for block number " +
              std::to_string(block_number));

  // end here
}

void TftpClient::CheckForSigintRRQ(Writer *writer) {
  if (SIGINT_RECEIVED.load() == false)
    return;

  auto error =
      ErrorPacket(ErrorPacket::ErrorCode::NOT_DEFINED, "User interrupted");
  udp_client_.Send(error.MakeRaw(), server_address_);

  auto path = writer->GetFilepath();
  writer->~Writer();

  FileHandler::DeleteFile(path);

  exit(EXIT_FAILURE);
}
