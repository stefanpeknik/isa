#include "TftpClient.h"

void TftpClient::Write() {
  SendWrq();
  SendData();
}

void TftpClient::SendWrq() {
  uint8_t retries = 0;
  while (retries < numRetries) {
    try {
      // send WRQ packet
      auto wrq = ReadWritePacket(TftpPacket::Opcode::WRQ, args_.dest_filepath,
                                 ReadWritePacket::Mode::OCTET, args_.options);
      Logger::Log("sending WRQ packet");
      udp_client_.Send(wrq.MakeRaw());
      // receive ACK / OACK / ERROR packet
      std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);
      auto response_1 = udp_client_.ReceiveFromAny(sender_address.get());
      // change port to the one that the first server response came from
      udp_client_.ChangePort(sender_address->sin_port);
      Logger::Log("New port: " + std::to_string(udp_client_.GetPort()));
      TftpPacket::Opcode opcode_1;
      // check if incomming packet is a correct TFTP packet
      opcode_1 = TftpPacket::GetOpcodeFromRaw(response_1);
      // check if opcode is ACK, OACK, or ERROR
      if (opcode_1 == TftpPacket::Opcode::ACK) {  // server responded with ACK
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
      } else if (opcode_1 == TftpPacket::Opcode::OACK) {  // server responded
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
      } else if (opcode_1 == TftpPacket::Opcode::ERROR) {  // server responded
                                                           // with ERROR
        Logger::Log("received ERROR packet");
        auto error = ErrorPacket(response_1);
        // TODO: client could send a new WRQ packet without options to retry
        throw TFTPIllegalOperationError(error.error_message);
      } else {
        throw TFTPIllegalOperationError("Expected ACK, OACK, or ERROR packet");
      }
      return;  // WRQ sent and ACK / OACK / ERROR received and handled
    } catch (UdpTimeoutException &e) {  // timeout
      retries++;                        // increment retry counter
      if (retries == numRetries) {
        throw;  // rethrow exception
      }
    }
    // every other exception is immidiately considered fatal and is thrown
  }
}

void TftpClient::SendData() {
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
  // loop that will continue until the last DATA packet containing less than
  // blksize bytes of data is sent or until the client receives an ERROR packet
  bool last_packet_sent = false;
  while (!last_packet_sent) {
    auto data = io_handler_.Read(blksize);
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
        DataPacket data_packet(block_number, data);
        Logger::Log("sending DATA packet");
        udp_client_.Send(data_packet.MakeRaw());
        auto response_n = udp_client_.ReceiveFromSpecific();
        TftpPacket::Opcode opcode_n = TftpPacket::GetOpcodeFromRaw(response_n);
        if (opcode_n == TftpPacket::Opcode::ACK) {  // ACK received
          auto ack_n = AckPacket(response_n);
          if (ack_n.block_number == block_number) {
            // ACK received for the correct block number, increment block
            // number
            ackReceived = true;
            block_number++;
          } else {
            // ACK received for the wrong block number, try again
            retries++;
            if (retries == numRetries) {
              throw TFTPIllegalOperationError(
                  "Expected ACK packet with block number " +
                  std::to_string(block_number) + ", got block number " +
                  std::to_string(ack_n.block_number));
            }
          }
        } else if (opcode_n == TftpPacket::Opcode::ERROR) {  // ERROR received
          auto error = ErrorPacket(response_n);
          throw TFTPIllegalOperationError(error.error_message);
        }
        // try again
      } catch (UdpTimeoutException &e) {  // timeout
        retries++;                        // increment retry counter
        if (retries == numRetries) {
          throw;  // rethrow exception
        }
      }  // every other exception is immidiately considered fatal and is thrown
    }
  }
}
