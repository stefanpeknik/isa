#include "TftpClient.h"

void TftpClient::Write() {
  SendWrq();
  SendData();
}

void TftpClient::SendWrq() {
  CheckForSigintWRQ();

  uint8_t retries = 0;
  try {
    // send WRQ packet
    auto wrq = ReadWritePacket(TftpPacket::Opcode::WRQ, args_.dest_filepath,
                               ReadWritePacket::Mode::OCTET, args_.options);
    Logger::Log("sending WRQ packet");
    udp_client_.Send(wrq.MakeRaw(), server_address_);
    // receive ACK / OACK / ERROR packet
    while (retries < MAX_RETRIES) {
      CheckForSigintWRQ();

      std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);
      std::vector<uint8_t> response_1;
      try {
        response_1 = udp_client_.Receive(sender_address.get());
      } catch (UdpSigintException &e) {
        CheckForSigintWRQ();
      }
      // change port to the one that the first server response came from
      server_address_.sin_port = sender_address->sin_port;
      Logger::Log("New port: " + std::to_string(server_address_.sin_port));
      LogPotentialTftpPacket(server_address_, response_1);
      // check if incomming packet is a correct TFTP packet
      auto opcode_1 = TftpPacket::GetOpcodeFromRaw(response_1);
      // check if opcode is ACK, OACK, or ERROR
      if (opcode_1 == TftpPacket::Opcode::ACK) { // server responded with ACK
        Logger::Log("received ACK packet");
        auto ack = AckPacket(response_1);
        if (ack.block_number != 0) {
          throw TFTPIllegalOperationError(
              "Expected ACK packet with block number 0, got "
              "block number " +
              std::to_string(ack.block_number));
        }
        // if client tried to send options, but server responded with ACK, then
        // server does not support options and client will move on without
        // options
        args_.options = {};
        // set up udp_client_ to use default block size and timeout (keep port)
        SetupUdpClient();
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
        SetupUdpClient();
      } else if (opcode_1 == TftpPacket::Opcode::ERROR) { // server responded
                                                          // with ERROR
        Logger::Log("received ERROR packet");
        auto error = ErrorPacket(response_1);
        // NOTE: client could send a new WRQ packet without options to retry
        throw RecievedErrorPacketException("Server responded with error: " +
                                           error.error_message);
      } else {
        throw TFTPIllegalOperationError("Expected ACK, OACK, or ERROR packet");
      }
      CheckForSigintWRQ();

      return; // WRQ sent and ACK / OACK / ERROR received and handled
    }
  } catch (UdpTimeoutException &e) { // timeout
    retries++;
    udp_client_.IncreaseTimeout();
  }
  // every other exception is immidiately considered fatal and is thrown

  if (retries >= MAX_RETRIES) {
    throw TFTPIllegalOperationError("Error: Timeout");
  }

  CheckForSigintWRQ();

  // reset timeout
  udp_client_.TimeoutReset();
}

void TftpClient::SendData() {
  CheckForSigintWRQ();

  // set up std handler
  StdHandler std_handler(args_.file_mode == ReadWritePacket::Mode::NETASCII
                             ? DataFormat::NETASCII
                             : DataFormat::OCTET);
  // send DATA packets
  // a loop that will continue until the last DATA packet containing less than
  // blksize bytes (or 'blksize' option value if specified) of data is sent or
  // until the client receives an ERROR packet
  uint16_t block_number = 1;
  // if 'blksize' option is specified, set blksize to the value of 'blksize'
  // option
  for (auto option : args_.options) {
    if (option.name == Option::Name::BLKSIZE) {
      blksize_ = stoi(option.value);
    }
  }
  // loop that will continue until the last DATA packet containing less than
  // blksize bytes of data is sent or until the client receives an ERROR packet
  bool last_packet_sent = false;
  while (!last_packet_sent) {
    CheckForSigintWRQ();

    std::vector<uint8_t> data;
    try {
      data = std_handler.ReadBytesFromStdin(blksize_);
    } catch (FailedToReadFromStdinException &e) {
      throw TFTPUnknownError();
    }
    if (data.size() < blksize_) {
      // last packet, set flag to true
      last_packet_sent = true;
    }
    // keep sending DATA packets until an ACK packet with the correct block
    // number is received or retry limit is reached
    int retries = 0;
    bool ackReceived = false;
    // create and send DATA packet (must be before loop because sorcerer's
    // apprenctice bug)
    auto data_packet = DataPacket(block_number, data, blksize_);
    Logger::Log("sending DATA packet with block number " +
                std::to_string(block_number));
    udp_client_.Send(data_packet.MakeRaw(), server_address_);
    while (!ackReceived && retries < MAX_RETRIES) {
      CheckForSigintWRQ();
      try {
        auto response_n = RecievePacketFromServer();
        auto opcode_n = TftpPacket::GetOpcodeFromRaw(response_n);
        if (opcode_n == TftpPacket::Opcode::ACK) { // ACK received
          auto ack_n = AckPacket(response_n);
          if (ack_n.block_number == block_number) {
            // ACK received for the correct block number, increment block
            // number
            ackReceived = true;
            block_number++;
          } else if (ack_n.block_number <=
                     block_number) { // ACK received for an old block
                                     // number, try again
            retries++;
            // increase timeout
            udp_client_.IncreaseTimeout();
          } else { // ACK received for a future block number, throw error
            throw TFTPIllegalOperationError(
                "Expected ACK packet with block number " +
                std::to_string(block_number) + ", got block number " +
                std::to_string(ack_n.block_number));
          }
        } else if (opcode_n == TftpPacket::Opcode::ERROR) { // ERROR received
          auto error = ErrorPacket(response_n);
          throw RecievedErrorPacketException("Server responded with error: " +
                                             error.error_message);
        }
        // try again
      } catch (UdpTimeoutException &e) { // timeout
        retries++;
        udp_client_.IncreaseTimeout();
      } catch (UdpSigintException &e) {
        CheckForSigintWRQ();
      } // every other exception is immidiately considered fatal and is thrown
      CheckForSigintWRQ();
    }
    CheckForSigintWRQ();

    if (retries >= MAX_RETRIES) {
      throw TFTPIllegalOperationError("Error: Timeout");
    }
    // reset timeout
    udp_client_.TimeoutReset();
  }
}

void TftpClient::CheckForSigintWRQ() {
  if (SIGINT_RECEIVED.load() == false)
    return;

  // send ERROR packet to server
  auto error =
      ErrorPacket(ErrorPacket::ErrorCode::NOT_DEFINED, "User interrupted");
  udp_client_.Send(error.MakeRaw(), server_address_);

  exit(EXIT_FAILURE);
}
