#include <cstdlib>
#include <iostream>
#include <string>

#include "TftpClient.h"

int FILEPATH_MAX_LENGTH = 400;

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
  exit(1);
}

TftpClient::TftpClientArgs ParseCommandLine(int argc, char *argv[]) {
  TftpClient::TftpClientArgs args;
  int i = 1;
  while (i < argc) {
    std::string arg = argv[i];
    if (arg == "-h") {
      i++;
      if (i < argc) {
        args.hostname = argv[i];
      } else {
        PrintUsageAndExit();
      }
    } else if (arg == "-p") {
      i++;
      if (i < argc) {
        args.port = std::atoi(argv[i]);
      } else {
        PrintUsageAndExit();
      }
    } else if (arg == "-f") {
      i++;
      if (i < argc) {
        args.filepath = argv[i];
      } else {
        PrintUsageAndExit();
      }
    } else if (arg == "-t") {
      i++;
      if (i < argc) {
        args.dest_filepath = argv[i];
        args.mode = TftpClient::TftpClientArgs::TftpMode::READ;
      } else {
        PrintUsageAndExit();
      }
    }
    i++;
  }

  if (args.hostname.empty() || args.dest_filepath.empty()) {
    PrintUsageAndExit();
  }

  // If port is not specified, use default port for TFTP
  if (args.port == 0) {
    args.port = 69;
  }

  // If filepath is not specified, read from stdin
  if (args.filepath.empty()) {
    std::getline(std::cin, args.filepath);
    args.mode = TftpClient::TftpClientArgs::TftpMode::WRITE;
  }

  return args;
}

int main(int argc, char *argv[]) {
  auto args = ParseCommandLine(argc, argv);
  args.options = {Option("timeout", "5")};

  auto client = TftpClient(args);

  try {
    client.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
