#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int check_terminate_fork(pid_t pid);
void handle_sigusr1(int sig)
{
    (void)sig;
    const char s[] = "[GATEWAY] Worker reported READY signal received\n";
    write(STDOUT_FILENO, (const void *)s, sizeof(s) - 1);
}

int main(void)
{
    if(signal(SIGUSR1, handle_sigusr1) != 0)
    {
        printf("[ERROR] FAIL INITIAL SIGUSR1\n");
        return 1;
    }
    pid_t pid = fork();

    if(pid < 0) {
        perror("fork fail");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        sleep(1);
        printf("[WORKER] Sent READY signal to gateway\n");
        kill(getppid(), SIGUSR1);
        exit(7);
    } 
    else {
        sigset_t new_mask;
        sigaddset(&new_mask, SIGUSR1);
        if (sigprocmask(SIG_BLOCK, &new_mask, NULL) < 0) {
            perror("sigprocmask");
            return -1;
        }
        sleep(5);
        if (sigprocmask(SIG_UNBLOCK, &new_mask, NULL) < 0) {
            perror("sigprocmask");
            return -1;
        }
    }
    check_terminate_fork(pid);
    return 0;
}

int check_terminate_fork(pid_t pid)
{
    int status;
    pid_t child_pid = waitpid(pid, &status, 0);
    if (child_pid <= 0) {
        perror("waitpid failed");
        return -1;
    }
    else {
        if(WIFEXITED(status) == 0) {
            fprintf(stderr, "Error: Worker process %d exit abnormally\n", pid);
        return -1;
        }
        else {
            int exit_code = WEXITSTATUS(status);
            if(exit_code != 0) {
                fprintf(stderr, "Error: Worker process %d Non-zero exit code:%d\n", pid, exit_code);
                return -1;
            }
        }
    }
    return 0;
}

