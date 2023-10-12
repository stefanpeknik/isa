#include <cstdlib>
#include <iostream>
#include <string>

#include "TftpClient.h"

void PrintUsage() {
  std::cerr << "Usage: tftpclient -h <hostname> -p <port> [-f <filepath>] -t "
               "<destination_filepath>\n"
               "-h <hostname> - The hostname of the server to connect to\n"
               "-p <port> - The port of the server to connect to\n"
               "-f <filepath> - The filepath to read from or write to\n"
               "-t <destination_filepath> - The filepath to write to or read "
               "from\n"
               "If -f is not specified, the filepath will be read from stdin\n";
  exit(1);
}

TftpClient::TftpClientArgs ParseCommandLine(int argc, char* argv[]) {
  TftpClient::TftpClientArgs args;
  int i = 1;
  while (i < argc) {
    std::string arg = argv[i];
    if (arg == "-h") {
      i++;
      if (i < argc) {
        args.hostname = argv[i];
      } else {
        PrintUsage();
      }
    } else if (arg == "-p") {
      i++;
      if (i < argc) {
        args.port = std::atoi(argv[i]);
      } else {
        PrintUsage();
      }
    } else if (arg == "-f") {
      i++;
      if (i < argc) {
        args.filepath = argv[i];
      } else {
        PrintUsage();
      }
    } else if (arg == "-t") {
      i++;
      if (i < argc) {
        args.dest_filepath = argv[i];
        args.mode = TftpClient::TftpClientArgs::TftpMode::READ;
      } else {
        PrintUsage();
      }
    }
    i++;
  }

  if (args.hostname.empty() || args.port == 0 || args.dest_filepath.empty()) {
    PrintUsage();
  }

  // If filepath is not specified, read from stdin
  if (args.filepath.empty()) {
    std::getline(std::cin, args.filepath);
    args.mode = TftpClient::TftpClientArgs::TftpMode::WRITE;
  }

  return args;
}

int main(int argc, char* argv[]) {
  auto client = TftpClient(ParseCommandLine(argc, argv));

  client.run();

  return 0;
}
