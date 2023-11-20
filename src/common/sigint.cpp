/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

#include "sigint.h"

std::atomic<bool> SIGINT_RECEIVED(false);

void SigintHandler(int signal) { SIGINT_RECEIVED.store(true); }
