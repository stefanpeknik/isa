#include "ClientHandler.h"

ClientHandler::ClientHandler(std::string hostname, int port,
                             std::string root_dirpath)
    : udp_client_(UdpClient()), root_dirpath_(root_dirpath) {
  // initialize the client address structure
  memset(&client_address_, 0, sizeof(client_address_));

  client_address_.sin_family = AF_INET; // IPv4
  // Set the IPv4 address directly
  inet_pton(AF_INET, hostname.c_str(), &client_address_.sin_addr);
  // set the port number
  client_address_.sin_port = htons(port);
}

void ClientHandler::FollowOnIntroPacket(std::vector<uint8_t> intro_packet) {
  LogPotentialTftpPacket(client_address_, intro_packet);
  // Parse the intro packet
  TftpPacket::Opcode opcode_1;
  try { // try to get the opcode
    opcode_1 = TftpPacket::GetOpcodeFromRaw(intro_packet);
  } catch (TFTPIllegalOperationError &e) {
    Logger::Log("Invalid opcode in intro packet\n");
    auto error = ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                             "Invalid opcode in intro packet");
    udp_client_.Send(error.MakeRaw(), client_address_);
    throw;
  }
  try { // try to follow on the intro packet and handle TFTP errors
    if (opcode_1 == TftpPacket::Opcode::RRQ) {
      FollowOnRRQ(intro_packet);
    } else if (opcode_1 == TftpPacket::Opcode::WRQ) {
      FollowOnWRQ(intro_packet);
    } else {
      throw TFTPIllegalOperationError("Expected RRQ or WRQ");
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
  // check if file exists
  if (!FileHandler::FileExists(read_file)) {
    throw TFTPFileNotFoundError("File does not exist: " + rrq.filepath);
  }
  // check if user is not trying to access files outside of the root directory
  if (!FileHandler::isFilePathUnderDirectory(read_file, root_dirpath_)) {
    throw TFTPAccessViolationError();
  }
  auto real_path = root_dirpath_ + "/" + rrq.filepath;
  auto read_mode = rrq.mode == ReadWritePacket::Mode::OCTET
                       ? DataFormat::OCTET
                       : DataFormat::NETASCII;
  Reader reader(real_path, read_mode);
  Logger::Log("read file: " + read_file);
  // update io handler to use the requested
  // negotiate options
  auto negotiated_options = NegotiateOptions(rrq.options);
  // if 'tsize' option is specified, read the file to get its size
  for (auto option : negotiated_options) {
    if (option.name == Option::Name::TSIZE) {
      Reader reader_for_tsize(real_path, read_mode);
      auto bytes_read = 0;
      do {
        auto data = reader_for_tsize.ReadFile(blksize_);
        bytes_read += data.size();
      } while (bytes_read == blksize_);
      option.value = std::to_string(bytes_read);
      // Reader destructor closes the file
    }
  }

  // if options were negotiated, send oack and get ack
  if (rrq.options.size() > 0) {
    Logger::Log("options were negotiated");
    // send oack
    auto oack = OackPacket(negotiated_options);
    Logger::Log("sending OACK packet");
    udp_client_.Send(oack.MakeRaw(), client_address_);
    // get ack
    int retries = 0;
    bool ackReceived = false;
    while (!ackReceived && retries < MAX_RETRIES) {
      try {
        auto packet = RecievePacketFromClient();
        auto opcode = TftpPacket::GetOpcodeFromRaw(packet);
        if (opcode == TftpPacket::Opcode::ACK) {
          auto ack = AckPacket(packet);
          if (ack.block_number != 0) {
            throw TFTPIllegalOperationError("Expected block number 0");
          }
          ackReceived = true;
          Logger::Log("received ACK packet with block number " +
                      std::to_string(ack.block_number));
        } else if (opcode == TftpPacket::Opcode::RRQ) {
          retries++;
          Logger::Log("received RRQ packet again, retry");
        } else if (opcode == TftpPacket::Opcode::ERROR) {
          auto error = ErrorPacket(packet);
          throw RecievedErrorPacketException("Client responded with error: " +
                                             error.error_message);
        } else {
          throw TFTPIllegalOperationError("Expected ACK or ERROR");
        }
      } catch (UdpTimeoutException &e) {
        retries++;
        udp_client_.IncreaseTimeout();
      } catch (UdpSigintException &e) {
        CheckForSigintRRQ(&reader);
      }
    }
    if (retries >= MAX_RETRIES) {
      throw TFTPIllegalOperationError("Error: Timeout");
    }
    // reset timeout
    udp_client_.TimeoutReset();
  }

  int block_number = 1;
  bool last_packet_sent = false;
  // keep sending data packets until the file is completely sent
  while (!last_packet_sent) {
    auto data = reader.ReadFile(blksize_); // read data from file
    if ((int)data.size() < blksize_) {
      // last packet, set flag to true
      Logger::Log("sending last packet, data size is less than blksize: " +
                  std::to_string(data.size()) + " < " +
                  std::to_string(blksize_));
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
    udp_client_.Send(data_packet.MakeRaw(), client_address_);
    while (!ackReceived && retries < MAX_RETRIES) {
      try {
        auto response_n = RecievePacketFromClient();
        auto opcode_n = TftpPacket::GetOpcodeFromRaw(response_n);
        if (opcode_n == TftpPacket::Opcode::ACK) { // ACK received
          auto ack_n = AckPacket(response_n);
          if (ack_n.block_number == block_number) {
            // ACK received for the correct block number, increment block
            // number
            Logger::Log("received ACK packet for block number " +
                        std::to_string(ack_n.block_number));
            ackReceived = true;
            block_number++;
          } else if (ack_n.block_number <=
                     block_number) { // ACK received for an old block
                                     // number, try again
            Logger::Log("received ACK packet for block number " +
                        std::to_string(ack_n.block_number) +
                        ", expected block number " +
                        std::to_string(block_number));
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
          throw RecievedErrorPacketException("Client responded with error: " +
                                             error.error_message);
        }
      } catch (UdpTimeoutException &e) { // timeout
        retries++;
        // increase timeout
        udp_client_.IncreaseTimeout();
      } catch (UdpSigintException &e) {
        CheckForSigintRRQ(&reader);
      }
    }
    if (retries >= MAX_RETRIES &&
        !last_packet_sent) { // timed out before sending last packet
      throw TFTPIllegalOperationError("Error: Timeout");
    }
    // if last packet was sent, we don't need to wait for an ACK and can simply
    // end here

    // reset timeout
    udp_client_.TimeoutReset();
  }
}

void ClientHandler::FollowOnWRQ(std::vector<uint8_t> intro_packet) {
  auto wrq = ReadWritePacket(intro_packet);
  Logger::Log("received WRQ packet");
  auto write_file = root_dirpath_ + "/" + wrq.filepath;
  Writer writer(root_dirpath_ + "/" + wrq.filepath,
                wrq.mode == ReadWritePacket::Mode::OCTET
                    ? DataFormat::OCTET
                    : DataFormat::NETASCII);
  // check if file exists
  if (FileHandler::FileExists(write_file)) {
    throw TFTPFileAlreadyExistsError("File already exists: " + wrq.filepath);
  }
  // check if user is not trying to access files outside of the root directory
  if (!FileHandler::isFilePathUnderDirectory(write_file, root_dirpath_)) {
    throw TFTPAccessViolationError();
  }
  Logger::Log("write file: " + write_file);
  // update io handler to use the requested
  // negotiate options
  auto negotiated_options = NegotiateOptions(wrq.options);

  int block_number = 1;
  int retries = 0;
  bool dataReceived = false;
  bool last_packet_received = false;

  for (auto option : negotiated_options) {
    if (option.name == Option::Name::TSIZE) {
      if (!FileHandler::HasEnoughSpace(root_dirpath_, this->tsize_)) {
        throw NotEnoughSpaceException();
      }
    }
  }

  // send ack or oack and recieve first data packet
  while (!dataReceived && retries < MAX_RETRIES) {
    try {
      // if options were negotiated, send oack
      if (wrq.options.size() > 0) {
        Logger::Log("options were negotiated");
        // send oack
        auto oack = OackPacket(negotiated_options);
        Logger::Log("sending OACK packet");
        udp_client_.Send(oack.MakeRaw(), client_address_);
      } else {
        // if no options were negotiated, send ack
        auto ack = AckPacket(0);
        Logger::Log("sending ACK packet with block number " +
                    std::to_string(block_number));
        udp_client_.Send(ack.MakeRaw(), client_address_);
      }
      // get data
      auto response_n = RecievePacketFromClient();
      auto opcode_n = TftpPacket::GetOpcodeFromRaw(response_n);
      if (opcode_n == TftpPacket::Opcode::DATA) { // DATA received
        auto data_n = DataPacket(response_n, blksize_);
        if (data_n.block_number == block_number) {
          // DATA received for the correct block number, increment block
          // number
          Logger::Log("received DATA packet with block number " +
                      std::to_string(data_n.block_number));
          dataReceived = true;
          // write data to file
          try {
            writer.WriteFile(data_n.data);
          } catch (FailedToWriteToFileException &e) {
            throw TFTPDiskFullError();
          } catch (FailedToOpenFileException &e) {
            throw TFTPAccessViolationError();
          }
          if ((int)data_n.data.size() < blksize_) {
            // last packet, set flag to true
            Logger::Log(
                "received last packet, data size is less than blksize: " +
                std::to_string(data_n.data.size()) + " < " +
                std::to_string(blksize_));
            last_packet_received = true;
          }
        } else if (data_n.block_number <=
                   block_number) { // DATA received for an old block
                                   // number, try again
          Logger::Log("received DATA packet with block number " +
                      std::to_string(data_n.block_number) +
                      ", expected block number " +
                      std::to_string(block_number + 1));
          retries++;
          // increase timeout
          udp_client_.IncreaseTimeout();
        } else { // DATA received for a future block number, throw error
          throw TFTPIllegalOperationError(
              "Expected DATA packet with block number " +
              std::to_string(block_number + 1) + ", got block number " +
              std::to_string(data_n.block_number));
        }
      } else if (opcode_n == TftpPacket::Opcode::ERROR) { // ERROR received
        auto error = ErrorPacket(response_n);
        throw RecievedErrorPacketException("Client responded with error: " +
                                           error.error_message);
      }
    } catch (UdpTimeoutException &e) {
      retries++;
      udp_client_.IncreaseTimeout();
    } catch (UdpSigintException &e) {
      CheckForSigintWRQ(&writer);
    }
  }
  if (retries >= MAX_RETRIES) {
    throw TFTPIllegalOperationError("Error: Timeout");
  }
  // reset timeout
  udp_client_.TimeoutReset();

  // keep receiving data packets until the file is completely received
  while (!last_packet_received) {
    retries = 0;
    dataReceived = false;
    while (!dataReceived && retries < MAX_RETRIES) {
      try {
        // send ACK packet
        auto ack = AckPacket(block_number);
        Logger::Log("sending ACK packet for block number " +
                    std::to_string(block_number));
        udp_client_.Send(ack.MakeRaw(), client_address_);
        // receive DATA packet
        auto response_n = RecievePacketFromClient();
        TftpPacket::Opcode opcode_n = TftpPacket::GetOpcodeFromRaw(response_n);
        if (opcode_n == TftpPacket::Opcode::DATA) { // DATA received
          auto data_n = DataPacket(response_n, blksize_);
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
            if ((int)data_n.data.size() < blksize_) {
              // last packet, set flag to true
              last_packet_received = true;
            }
            break;
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
        if (retries >= MAX_RETRIES) {
          throw TFTPIllegalOperationError("Error: Timeout");
        }
      } catch (UdpSigintException &e) {
        CheckForSigintWRQ(&writer);
      }
    }
    if (retries >= MAX_RETRIES) {
      throw TFTPIllegalOperationError("Error: Timeout");
    }
    // reset timeout
    udp_client_.TimeoutReset();
  }
  // as a good manner send last ACK for last DATA
  auto ack = AckPacket(block_number);
  udp_client_.Send(ack.MakeRaw(), client_address_);
  Logger::Log("sending ACK packet for block number " +
              std::to_string(block_number));

  // end here
}

std::vector<Option>
ClientHandler::NegotiateOptions(std::vector<Option> options) {
  std::vector<Option> negotiated_options;
  for (auto option : options) {
    switch (option.name) {
    case Option::Name::BLKSIZE:
      this->blksize_ = stoi(option.value);
      negotiated_options.push_back(option);
      break;
    case Option::Name::TIMEOUT:
      this->timeout_ = {stoi(option.value), 0};
      udp_client_.ChangeTimeout(this->timeout_);
      negotiated_options.push_back(option);
      break;
    case Option::Name::TSIZE:
      // has to be implemented outside of this function
      this->tsize_ = stoi(option.value);
      negotiated_options.push_back(option);
      break;
    case Option::Name::UNSUPPORTED: // ignore
      break;
    }
  }
  return negotiated_options;
}

void ClientHandler::CheckForSigintWRQ(Writer *writer) {
  if (SIGINT_RECEIVED.load() == false)
    return;
  Logger::Log("SIGINT received, terminating transfer");
  // send error packet
  auto error = ErrorPacket(ErrorPacket::ErrorCode::NOT_DEFINED,
                           "User interrupted transfer");
  udp_client_.Send(error.MakeRaw(), client_address_);
  auto path = writer->GetFilepath();
  // destructor closes file
  writer->~Writer();
  // delete file
  FileHandler::DeleteFile(path);

  exit(EXIT_FAILURE);
}

void ClientHandler::CheckForSigintRRQ(Reader *reader) {
  if (SIGINT_RECEIVED.load() == false)
    return;
  Logger::Log("SIGINT received, terminating transfer");
  // send error packet
  auto error = ErrorPacket(ErrorPacket::ErrorCode::NOT_DEFINED,
                           "User interrupted transfer");
  udp_client_.Send(error.MakeRaw(), client_address_);
  auto path = reader->GetFilepath();
  // destructor closes file
  reader->~Reader();
  // delete file
  FileHandler::DeleteFile(path);

  exit(EXIT_FAILURE);
}

std::vector<uint8_t> ClientHandler::RecievePacketFromClient() {
  std::vector<uint8_t> buffer;

  // Create a unique_ptr to a sockaddr_in object to store the sender's
  // address
  std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);

  // Receive data from any sender and capture the sender's address
  buffer = udp_client_.Receive(sender_address.get());

  int retries = 0;
  while ((sender_address->sin_addr.s_addr !=
              client_address_.sin_addr.s_addr ||                   // ip
          sender_address->sin_port != client_address_.sin_port) && // port
         retries < MAX_RETRIES) {                                  // retries
    // Keep receiving data until the sender's port matches the client's
    // port or the number of retries exceeds the maximum number of retries

    // Log the received packet if it is a valid TFTP packet
    LogPotentialTftpPacket(*sender_address.get(), buffer);

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

  if (retries >= MAX_RETRIES) {
    throw UdpTimeoutException();
  }

  // Log the received packet if it is a valid TFTP packet
  LogPotentialTftpPacket(*sender_address.get(), buffer);

  return buffer;
}

void ClientHandler::LogPotentialTftpPacket(struct sockaddr_in sender_address,
                                           std::vector<uint8_t> buffer) {
  // Log the received packet if it is a valid TFTP packet
  try {
    auto opcode = TftpPacket::GetOpcodeFromRaw(buffer);
    if (opcode == TftpPacket::Opcode::RRQ) {
      Logger::LogRRQ(sender_address, ReadWritePacket(buffer));
    } else if (opcode == TftpPacket::Opcode::WRQ) {
      Logger::logWRQ(sender_address, ReadWritePacket(buffer));
    } else if (opcode == TftpPacket::Opcode::DATA) {
      Logger::logDATA(sender_address, udp_client_.GetLocalPort(),
                      DataPacket(buffer, blksize_));
    } else if (opcode == TftpPacket::Opcode::ACK) {
      Logger::logACK(sender_address, AckPacket(buffer));
    } else if (opcode == TftpPacket::Opcode::ERROR) {
      Logger::logERROR(sender_address, udp_client_.GetLocalPort(),
                       ErrorPacket(buffer));
    } else if (opcode == TftpPacket::Opcode::OACK) {
      Logger::logOACK(sender_address, OackPacket(buffer));
    } else {
      // invalid opcode
    }
  } catch (const std::exception &) {
    // ignore any exceptions
  }
}