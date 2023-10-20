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
  auto rrq = ReadWritePacket(intro_packet);
  
}

void ClientHandler::FollowOnWRQ(std::vector<uint8_t> intro_packet) {}