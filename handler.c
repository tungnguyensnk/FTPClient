#include "handler.h"

void signal_handler(int signum) {
    printf("Signal: %d\n", signum);
    if (signum == SIGINT) {
        exit(0);
    } else if (signum == SIGCHLD) {
        int stat = 0;
        while (waitpid(-1, &stat, WNOHANG) > 0) {

        }
    }
}
