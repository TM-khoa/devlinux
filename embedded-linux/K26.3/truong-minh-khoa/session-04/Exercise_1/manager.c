#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#define FORK_NUM 3
#define AVAIL_ORDERS 3
#define NAME_LENGTH 50

typedef struct {
    int   id;
    char  name[NAME_LENGTH];
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

int process_order(int order_index) {
    if(order_index < 0 || order_index >= AVAIL_ORDERS) {
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

int main()
{
    printf("===================================================\n");
    printf("   ORDER PROCESSING SYSTEM — MANAGER (fork+wait)\n");
    printf("===================================================\n");
    printf("[MANAGER] PID: %d — spawning 3 child processes...\n", getpid());
    for(int i = 0; i < FORK_NUM; i++) {
        fflush(stdout);
        child_pids[i] = fork();

        if(child_pids[i] < 0){
            perror("fork fail");
            exit(1);
        }

        if (child_pids[i] == 0) {
            /* Save child PID for the next check status */
            if(process_order(i) != 0)
            {
                printf("Fail to get order:%d\n", i);
                exit(1);
            }
            exit(0);
        }
        printf("[MANAGER] fork() order #%d → child PID: %d\n", i + 1, child_pids[i]);
    }
    printf("[MANAGER] All 3 children spawned. Starting waitpid()...\n");
    printf("--- [child output order may interleave — this is normal] ---\n");

    for(int i = 0; i < FORK_NUM; i++) {
        int status;
        if (child_pids[i] <= 0) {
            continue;
        }

        if (waitpid(child_pids[i], &status, 0) < 0) {
            perror("waitpid");
            order_fail++;
            continue;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            total_revenue += orders[i].quantity * orders[i].unit_price;;
            order_success++;
            printf("[MANAGER] waitpid(%d) — order %d#: exit code=%d → SUCCESS\n",
                    child_pids[i], orders[i].id, WEXITSTATUS(status));
        } else if (WIFEXITED(status)) {
            printf("[MANAGER] waitpid(%d) — order %d#: exit code=%d → FAILED\n", 
                    child_pids[i], orders[i].id, WEXITSTATUS(status));
            order_fail++;
        }
        else {
            printf("[MANAGER] waitpid(%d) — order %d#:child did not exit normally\n", 
                    child_pids[i], orders[i].id);
            order_fail++;
        }
    }

    printf("\n================= SUMMARY =================\n");
    printf("\t Total orders : %d \n", order_success + order_fail);
    printf("\t Successful   : %d \n", order_success);
    printf("\t Failed       : %d \n", order_fail);
    printf("\t Total revenue: %.0f VND \n", total_revenue);
    printf("===========================================\n");
    return 0;
}
