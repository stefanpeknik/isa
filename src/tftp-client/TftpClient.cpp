#include "TftpClient.h"

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
    
}

void TftpClient::Read() {}
