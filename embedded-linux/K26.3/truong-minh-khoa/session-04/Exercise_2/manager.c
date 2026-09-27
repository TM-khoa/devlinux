#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>



int main()
{
    char banner[] = 
"===================================================\n\
\tSTUDENT LOOKUP SYSTEM — MANAGER \n\
\t(fork + execve | file: students.txt)\n\
===================================================\n\
[MANAGER] PID: %d \n\
Enter Student ID ('quit' to exit)\n\
";
    pid_t child_pids = {0};
    char student_id[10]= {0};
    printf(banner, getpid());
    while(1) {
        printf("---------------------------------------------\n");
        printf("Student ID: ");
        if(scanf("%s", student_id) != 1) {
            printf("Error: Invalid input argument\n");
            return 1;
        }
        if(strcmp(student_id, "quit") == 0) {
            printf("[MANAGER] Exiting. Goodbye!");
            break;
        }
        pid_t pid = fork();
        fflush(stdout);
        if(pid < 0){
            perror("fork fail");
            exit(1);
        }
        if (pid == 0) {
            const char *binaryPath = "./searcher";
            char *const args[] = {"./searcher", student_id, "students.txt", NULL};
            execve(binaryPath, args, NULL);
            exit(0);
        }
        else {
            printf("[MANAGER] fork() → child PID:%d\n", pid);
        }
        int status;
        printf("[MANAGER] Waiting for child (waitpid)...\n");
        pid_t cpid = waitpid(child_pids, &status, 0); 
        if (cpid > 0) {
            if (WIFEXITED(status)) {
                printf("[MANAGER] Child (PID %d) exited. code=%d → %s\n", cpid, WEXITSTATUS(status), status == 0 ? "Found" : "Not found");
            } else if (WIFSIGNALED(status)) {
                printf("[MANAGER] waitpid(%d) exit code=%d → FAILED\n", 
                        cpid, WTERMSIG(status));
            }
        }
    }
    return 0;
}
