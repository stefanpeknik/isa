#include "../common/TftpPacketStuff/TftpCommon.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "TftpClient.h"

long unsigned int FILEPATH_MAX_LENGTH =
    500; // to fit in default max packet size of 516 bytes

void PrintUsageAndExit() {
  std::cout
      << "Usage:\n"
         "tftp-client -h hostname [-p port] [-f filepath] -t dest_filepath\n\n"
         "-h Remote server's IP address/domain name\n"
         "-p Remote server's port\n"
         "If not specified, the default port is assumed per specification\n"
         "-f Path to the file to be downloaded from the server (download)\n"
         "If not specified, data will be read from stdin (upload)\n"
         "-t Path where the file will be stored on the remote server or "
         "locally\n";
  exit(EXIT_FAILURE);
}

// TODO : fix error codes for options
// if unsupported option is received
// TODO : netascii must end with \r\n

int main(int argc, char *argv[]) {
  std::string hostname;
  int port;
  std::string filepath = "";
  std::string dest_filepath;
  ReadWritePacket::Mode file_mode = ReadWritePacket::Mode::OCTET; // hardcoded
  TftpClient::TftpClientArgs::TftpMode mode;
  std::vector<Option> options = {};

  int i = 1;
  while (i < argc) {
    std::string arg = argv[i];
    if (arg == "-h") {
      i++;
      if (i < argc) {
        hostname = argv[i];
      } else {
        PrintUsageAndExit();
      }
    } else if (arg == "-p") {
      i++;
      if (i < argc) {
        port = std::atoi(argv[i]);
      } else {
        PrintUsageAndExit();
      }
    } else if (arg == "-f") {
      i++;
      if (i < argc) {
        filepath = argv[i];
      } else {
        PrintUsageAndExit();
      }
    } else if (arg == "-t") {
      i++;
      if (i < argc) {
        dest_filepath = argv[i];
      } else {
        PrintUsageAndExit();
      }
    }
    i++;
  }

  if (hostname.empty() || dest_filepath.empty()) {
    PrintUsageAndExit();
  }

  // If filepath is not specified, Write
  if (filepath.empty()) {
    if (filepath.length() > FILEPATH_MAX_LENGTH) {
      std::cerr << "Error: Filepath is too long" << std::endl;
      exit(EXIT_FAILURE);
    }
    mode = TftpClient::TftpClientArgs::TftpMode::WRITE;
  } else {
    mode = TftpClient::TftpClientArgs::TftpMode::READ;
  }

  // If port is not specified, use default port for TFTP
  if (port == 0) {
    port = 69;
  }

  options = {Option("timeout", "10")};
  // args.options = {};

  auto client = TftpClient(TftpClient::TftpClientArgs{
      hostname, port, filepath, dest_filepath, mode, file_mode, options});
  try {
    client.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
