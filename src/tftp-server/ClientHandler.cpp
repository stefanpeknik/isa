#include "ClientHandler.h"

ClientHandler::ClientHandler(std::string hostname, int port)
    : udp_client_(UdpClient(hostname, port)),
      io_handler_(IOHandler(IOHandler::IO::FILE)) {}

void ClientHandler::FollowOnIntroPacket(std::vector<uint8_t> intro_packet) {
  // Parse the intro packet
  TftpPacket::Opcode opcode_1;
  try {
    opcode_1 = TftpPacket::GetOpcodeFromRaw(intro_packet);
  } catch (TFTPIllegalOperationError &e) {
    Logger::Log("Invalid opcode in intro packet\n");
    auto error = ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION,
                             "Invalid opcode in intro packet");
    udp_client_.Send(error.MakeRaw());
    throw;
  }

  if (opcode_1 == TftpPacket::Opcode::RRQ) {
    FollowOnRRQ(intro_packet);
  } else if (opcode_1 == TftpPacket::Opcode::WRQ) {
    FollowOnWRQ(intro_packet);
  }
}

void ClientHandler::FollowOnRRQ(std::vector<uint8_t> intro_packet) {
  try {
    auto rrq = ReadWritePacket(intro_packet);
    // check if file exists
    if (!io_handler_.FileExists(rrq.filepath)) {
      throw TFTPFileNotFoundError("File does not exist");
    }
    // open file
    try {
      io_handler_.OpenFile(rrq.filepath, std::ios_base::binary);
    } catch (FailedToOpenFileException &e) {
      throw TFTPAccessViolationError();
    }
    if (rrq.options.size() > 0) {
      // negotiate options
      
    }

  } catch (TFTPFileNotFoundError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::FILE_NOT_FOUND, e.what());
    udp_client_.Send(error.MakeRaw());
    throw;
  } catch (TFTPAccessViolationError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::ACCESS_VIOLATION, e.what());
    udp_client_.Send(error.MakeRaw());
    throw;
  } catch (TFTPDiskFullError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::DISK_FULL, e.what());
    udp_client_.Send(error.MakeRaw());
    throw;
  } catch (TFTPIllegalOperationError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::ILLEGAL_OPERATION, e.what());
    udp_client_.Send(error.MakeRaw());
    throw;
  } catch (TFTPUnknownTransferIDError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::UNKNOWN_TID, e.what());
    udp_client_.Send(error.MakeRaw());
    throw;
  } catch (TFTPFileAlreadyExistsError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::FILE_ALREADY_EXISTS, e.what());
    udp_client_.Send(error.MakeRaw());
    throw;
  } catch (TFTPNoSuchUserError &e) {
    auto error = ErrorPacket(ErrorPacket::ErrorCode::NO_SUCH_USER, e.what());
    udp_client_.Send(error.MakeRaw());
    throw;
  } catch (TTFOptionNegotiationError &e) {
    auto error =
        ErrorPacket(ErrorPacket::ErrorCode::FAILED_NEGOTIATION, e.what());
    udp_client_.Send(error.MakeRaw());
    throw;
  }
}

void ClientHandler::FollowOnWRQ(std::vector<uint8_t> intro_packet) {}