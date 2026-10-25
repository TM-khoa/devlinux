#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t keep_running = 1;


void handle_sigterm(int sig) {
    (void)sig;
    keep_running = 0;
}

int main()
{
    /* Not line buffer with printf, printout immediately to journal */
    setbuf(stdout, NULL);
    signal(SIGTERM, handle_sigterm);
    printf("Monitor service start\n");
    while (keep_running) {
        printf("Monitoring heartbeat\n");
        sleep(1);
    }
    printf("Monitor service stop\n");
    return 0;
}



