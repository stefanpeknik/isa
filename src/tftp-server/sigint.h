#ifndef Sigint_h
#define Sigint_h

#include <csignal>
#include <atomic>

// Global variable to indicate that SIGINT was received
std::atomic<bool> SIGINT_RECEIVED(false);

void SigintHandler(int signal);

#endif // Sigint_h
