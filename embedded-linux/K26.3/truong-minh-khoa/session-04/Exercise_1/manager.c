#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#define FORK_NUM 3
#define AVAIL_ORDERS 3

typedef struct {
    int   id;
    char  name[50];
    int   quantity;
    float unit_price;
} Order;

Order orders[AVAIL_ORDERS] = {
    {1, "Backpack", 2, 350000},
    {2, "Shoes",    1, 500000},
    {3, "Hat",      3, 120000},
};

static pid_t child_pids[FORK_NUM] = {0};
static int order_success = 0;
static int order_fail = 0;
static float total_revenue = 0.0;
static char output[60] = {0};

int process_order(int order_index) {
    if(order_index < 0 || order_index > AVAIL_ORDERS) {
        printf("Error: Out of range available order\n");
        return 1;
    }
    Order o = orders[order_index];
    float total = o.quantity * o.unit_price;
    printf("[CHILD-%d] PID: %d | PPID: %d\n", o.id, getpid(), getppid());
    printf("[CHILD-%d] %s x%d — Total: %.0f VND\n",
            o.id, o.name, o.quantity, total);
    printf("[CHILD-%d] Processing... (sleep 2s)\n\n", o.id);

    sleep(2);
    return 0;
}

char* format_currency(int number, char *output) {
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%d", number);  // Convert number to string

    int len = strlen(buffer);
    int commas = (len - 1) / 3;     // Number of commas needed
    int new_len = len + commas;

    output[new_len] = '\0';         // Null-terminate result

    int i = len - 1;                // Index in buffer
    int j = new_len - 1;            // Index in output
    int count = 0;

    while (i >= 0) {
        output[j--] = buffer[i--];
        count++;
        if (count == 3 && i >= 0) {
            output[j--] = ',';
            count = 0;
        }
    }
    return output;
}

int main()
{
    printf("===================================================\n");
    printf("   ORDER PROCESSING SYSTEM — MANAGER (fork+wait)\n");
    printf("===================================================\n");
    printf("[MANAGER] PID: %d — spawning 3 child processes...\n", getpid());
    for(int i = 0; i < FORK_NUM; i++) {
        fflush(stdout);
        pid_t pid = fork();
        if(pid < 0){
            perror("fork fail");
            exit(1);
        }
        if (pid == 0) {
            sleep(1);
            /* Save child PID for the next check status */
            if(process_order(i) != 0)
            {
                printf("Fail to get order:%d\n", i);
                exit(1);
            }
            exit(0);
        }
        else {
            child_pids[i] = pid;
            printf("[MANAGER] fork() order #%d → child PID: %d\n", i + 1, pid);
        }
    }
    printf("[MANAGER] All 3 children spawned. Starting waitpid()...\n");
    printf("--- [child output order may interleave — this is normal] ---\n");

    for(int i = 0; i < FORK_NUM; i++) {
        int status;
        pid_t cpid = waitpid(child_pids[i], &status, 0); 
        if (cpid > 0) {
            if (WIFEXITED(status)) {
                float total = orders[i].quantity * orders[i].unit_price;
                total_revenue += total;
                order_success++;
                printf("[MANAGER] waitpid(%d) — order %d#: exit code=%d → SUCCESS\n",
                         cpid, i + 1, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("[MANAGER] waitpid(%d) — order %d#: exit code=%d → FAILED\n", 
                         cpid, i + 1, WTERMSIG(status));
                order_fail++;
            }
        }
    }

    printf("\n================= SUMMARY =================\n");
    printf("\t Total orders : %d \n", order_success + order_fail);
    printf("\t Successful   : %d \n", order_success);
    printf("\t Failed       : %d \n", order_fail);
    printf("\t Total revenue: %s VND \n", format_currency((int)total_revenue, output));
    printf("===========================================\n");
    return 0;
}
