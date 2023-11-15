#ifndef GLOBALS_H
#define GLOBALS_H

#include <atomic>

extern std::atomic<bool> SIGINT_RECEIVED;

void SigintHandler(int signal);

#endif // GLOBALS_H
