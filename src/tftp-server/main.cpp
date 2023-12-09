/**
 * Author: Stefan Peknik
 * Mail: xpekni01@vutbr.cz
 */

#include <cstdlib>
#include <iostream>
#include <string>

#include "TftpServer.h"

void PrintUsageAndExit() {
  std::cout << "Usage: tftp-server [-p port] root_dirpath" << std::endl;
  std::cout << "-p\t\tLocal port on which the server will expect incoming "
               "connections"
            << std::endl;
  std::cout << "root_dirpath\tPath to the directory where incoming files "
               "will be stored"
            << std::endl;
  exit(1);
}

TftpServer::TftpServerArgs ParseCommandLine(int argc, char *argv[]) {
  if (argc < 3 || argc > 4)
    PrintUsageAndExit();
  TftpServer::TftpServerArgs args;
  int i = 1;
  while (i < argc) {
    std::string arg = argv[i];
    if (arg == "-p") {
      i++;
      if (i < argc) {
        args.port = std::atoi(argv[i]);
      } else {
        PrintUsageAndExit();
      }
    } else {
      args.root_dirpath = argv[i];
    }
    i++;
  }

  if (args.root_dirpath.empty())
    PrintUsageAndExit();

  return args;
}

int main(int argc, char *argv[]) {
  auto args = ParseCommandLine(argc, argv);

  auto server = TftpServer(args);

  try {
    server.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
