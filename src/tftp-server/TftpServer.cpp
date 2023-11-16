#include "TftpServer.h"

TftpServer::TftpServer(TftpServerArgs args)
    : args_(args), udp_server_(UdpServer(args.port)) {
  // Set up the SIGINT handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SigintHandler;
  sigaction(SIGINT, &sa, NULL);

  // remove SA_RESTART from the SIGINT handler
  sa.sa_flags &= ~SA_RESTART;
  sigaction(SIGINT, &sa, NULL);
}

void TftpServer::run() {
  std::vector<std::thread> client_threads;

  while (SIGINT_RECEIVED.load() == false) {
    std::vector<uint8_t> data;
    struct sockaddr_in sender_address;
    // Wait for a packet to arrive
    try {
      while (!udp_server_.Receive(data, sender_address)) {
        if (SIGINT_RECEIVED.load() == true)
          break; // If SIGINT was received while waiting, break out of the
                 // receiving loop
      }
    } catch (UdpSigintException &e) {
    }
    if (SIGINT_RECEIVED.load() == true)
      break; // If SIGINT was received while waiting, break out of the loop

    std::string client_hostname = inet_ntoa(sender_address.sin_addr);
    int client_port = ntohs(sender_address.sin_port);

    client_threads.push_back(std::thread(&TftpServer::StartCommsWithClient,
                                         this, client_hostname, client_port,
                                         args_.root_dirpath, data));
  }
  if (SIGINT_RECEIVED.load() == true) {
    for (std::thread &thread : client_threads) {
      pthread_kill(thread.native_handle(), SIGUSR1);
    }
  }
}

void TftpServer::StartCommsWithClient(std::string client_hostname,
                                      int client_port, std::string root_dirpath,
                                      std::vector<uint8_t> intro_packet) {
  try {
    auto client_handler =
        ClientHandler(client_hostname, client_port, root_dirpath);
    client_handler.FollowOnIntroPacket(intro_packet);
  } catch (const std::exception &e) {
    Logger::Log(e.what());
  }
}
