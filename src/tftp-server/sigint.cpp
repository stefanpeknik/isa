#include "sigint.h"

void SigintHandler(int signal) { SIGINT_RECEIVED.store(true); }
