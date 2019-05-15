// File: signals.c
// Author: iBug

#include "signals.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void register_signal_handlers(void) {
    signal(SIGINT, on_SIGINT);
}

void on_SIGINT(int signal) {
    fprintf(stderr, "Shutting down...\n");
    _exit(0);
}
