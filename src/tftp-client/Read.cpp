#include "TftpClient.h"

void TftpClient::Read() {
  // declare block number
  // - if client sends RRQ with options, but server responds with DATA, then
  // server does not support options and client will move on without options
  // send ACK packet with block number 1
  // - if client sends RRQ with options, and server responds with OACK, then
  // client will respond with ACK packet with block number 0
  uint16_t block_number;
  uint8_t retries = 0;
  while (retries < numRetries) {
    try {
      // send RRQ packet
      auto rrq = ReadWritePacket(TftpPacket::Opcode::RRQ, args_.dest_filepath,
                                 ReadWritePacket::Mode::OCTET, args_.options);
      Logger::Log("sending RRQ packet");
      udp_client_.Send(rrq.MakeRaw());
      // receive DATA / OACK / ERROR packet
      std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);
      auto response_1 = udp_client_.ReceiveFromAny(sender_address.get());
      // change port to the one that the first server response came from
      udp_client_.ChangePort(sender_address->sin_port);
      Logger::Log("New port: " + std::to_string(udp_client_.GetPort()));
      TftpPacket::Opcode opcode_1;
      // check if incomming packet is a correct TFTP packet
      opcode_1 = TftpPacket::GetOpcodeFromRaw(response_1);
      // check if opcode is DATA, OACK, or ERROR
      if (opcode_1 == TftpPacket::Opcode::DATA) {  // server responded with DATA
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
        // write data to file
        io_handler_.Write(data.data);
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
        // set block number to 0
        block_number = 0;
      } else if (opcode_1 == TftpPacket::Opcode::ERROR) {  // server responded
                                                           // with ERROR
        Logger::Log("received ERROR packet");
        auto error = ErrorPacket(response_1);
        // TODO: client could send a new RRQ packet without options to retry
        throw TFTPIllegalOperationError(error.error_message);
      } else {
        auto error = ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                                 "Expected DATA, OACK, or ERROR packet");
        udp_client_.Send(error.MakeRaw());
        throw TFTPIllegalOperationError("Expected DATA, OACK, or ERROR packet");
      }
    } catch (UdpTimeoutException &e) {  // timeout
      retries++;
      if (retries == numRetries) {
        throw TFTPIllegalOperationError("Error: Timeout");
      }
    } catch (NotEnoughSpaceOnDiskException &e) {
      throw TFTPDiskFullError();
    }

    // so far, client has sent RRQ packet and received DATA / OACK

    // define blksize
    uint16_t blksize = 512;
    // if 'blksize' option is specified, set blksize to the value of 'blksize'
    // option
    for (auto option : args_.options) {
      if (option.name == Option::Name::BLKSIZE) {
        blksize = stoi(option.value);
      }
    }
    // send ACKs and receive DATA packets
    // a loop that will continue until the last DATA packet containing less
    // than 512 bytes (or 'blksize' option value if specified) of data is
    // received or until the client receives an ERROR packet
    bool last_packet_received = false;
    while (!last_packet_received) {
      retries = 0;
      auto dataReceived = false;
      while (!dataReceived && retries < numRetries) {
        try {
          // send ACK packet
          auto ack = AckPacket(block_number);
          Logger::Log("sending ACK packet for block number " +
                      std::to_string(block_number));
          udp_client_.Send(ack.MakeRaw());
          // receive DATA packet
          auto response_n = udp_client_.ReceiveFromSpecific();
          TftpPacket::Opcode opcode_n =
              TftpPacket::GetOpcodeFromRaw(response_n);
          if (opcode_n == TftpPacket::Opcode::DATA) {  // DATA received
            auto data_n = DataPacket(response_n);
            if (data_n.block_number == block_number + 1) {
              // DATA received for the correct block number, increment block
              // number
              block_number++;
              Logger::Log("received DATA packet for block number " +
                          std::to_string(data_n.block_number));
              // write data to file
              io_handler_.Write(data_n.data);
              // check if this is the last DATA packet
              if (data_n.data.size() < blksize) {
                // last packet, set flag to true
                last_packet_received = true;
              }
            } else {
              // DATA received for the wrong block number, try again
              Logger::Log("received DATA packet for the wrong block number");
              retries++;
              if (retries == numRetries) {
                throw TFTPIllegalOperationError(
                    "Expected DATA packet with block number " +
                    std::to_string(block_number + 1) + ", got block number " +
                    std::to_string(data_n.block_number));
              }
            }
          } else if (opcode_n == TftpPacket::Opcode::ERROR) {  // ERROR received
            Logger::Log("received ERROR packet");
            auto error = ErrorPacket(response_n);
            throw TFTPIllegalOperationError(error.error_message);
          }
          // try again
        } catch (UdpTimeoutException &e) {  // timeout
          retries++;
          if (retries == numRetries) {
            throw;
          }
        } catch (NotEnoughSpaceOnDiskException &e) {
          throw TFTPDiskFullError();
        }
      }
    }
  }
}
