#include "handler.h"

// hàm xử lý các signal
void signal_handler(int signum) {
    if (signum == SIGINT) {
        exit(0);
    } else if (signum == SIGCHLD) {
        int stat = 0;
        while (waitpid(-1, &stat, WNOHANG) > 0) {

        }
    }
}
