#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define QUEUE_MAX_SIZE 5
#define FILE_NAME_LENGTH 60
#define SLEEP_DELAY 1

typedef struct {
    int  doc_id;
    char filename[FILE_NAME_LENGTH];
    int  pages;
} Document;

#define THREAD_PRINTER_NUM 1
#define THREAD_PRODUCER_NUM 3

Document queue[QUEUE_MAX_SIZE];
int head = 0, tail = 0, count = 0;
int all_sent = 0;           /* set to 1 by main after joining all producers */

pthread_mutex_t q_lock;
pthread_cond_t  not_full;   /* producers wait here when count == 5 */
pthread_cond_t  not_empty;  /* printer  waits here when count == 0 */

const char *banner =
"==============================================\n\
   OFFICE PRINT QUEUE (3 producers, 1 printer)\n\
   Queue capacity: 5 documents\n\
==============================================\n";

char summary_log[] = 
"================ SUMMARY ================\n\
Documents submitted : %d\n\
Documents printed   : %d\n\
Total pages printed : %d\n\
=========================================\n";

int thread_id[3] = {0, 1, 2};
int doc_submitted = 0;
int doc_printed = 0;
int total_pages = 0;
Document doc_list[] = {
    {1,"report_Q1.pdf",12},
    {2,"contract.pdf",5},
    {3,"memo.pdf",2},
    {4,"proposal.pdf",8},
    {5,"invoice.pdf",3},
    {6,"budget.pdf",7},
    {7,"slides.pdf",20},
    {8,"summary.pdf",4},
    {9,"dummy.pdf", 14},
};
void* producer(void *arg);
void* printer(void *arg);
void print_summary();
int dequeue(Document *doc);
int enqueue(Document doc);


int main()
{
    pthread_t thread_producer[THREAD_PRODUCER_NUM];
    pthread_t thread_printer;
    int result;
    printf("%s", banner);
    pthread_mutex_init(&q_lock, NULL);

    result = pthread_create(&thread_printer, NULL, printer, NULL);
    if (result != 0) {
        perror("Thread creation failed");
        return 1;
    }

    for(int i = 0; i < THREAD_PRODUCER_NUM;i++) {
        result = pthread_create(&thread_producer[i], NULL, producer, (void*)&thread_id[i]);
        if (result != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }

    for(int i = 0; i < THREAD_PRODUCER_NUM;i++) {
        int err = pthread_join(thread_producer[i], NULL);
        if(err != 0) {
            printf("Error: Fail to join the thread\n");
        }
    }
    printf("all producer are joined\n");
    all_sent = 1;
    pthread_cond_broadcast(&not_empty);
    int err = pthread_join(thread_printer, NULL);
    if(err != 0) {
        printf("Error: Fail to join the thread\n");
    }
    pthread_mutex_destroy(&q_lock);
    print_summary();
    return 0;
}


void* producer(void *arg)
{
    int thread_id = 0;
    if(arg != NULL) {
        thread_id = *(int *)arg;
    }
    else {
        thread_id = 0;
    }

    for(int i = 0; i < 3; i++) {
        pthread_mutex_lock(&q_lock);
        Document doc = doc_list[i + (3 * thread_id)];
        while (count == QUEUE_MAX_SIZE) {
            pthread_cond_wait(&not_full, &q_lock);
        }
        enqueue(doc);
        doc_submitted++;
        printf("[Producer %d] Submitting:%s (%d pages) - queue: %d/%d\n", 
                thread_id, 
                doc.filename, 
                doc.pages, 
                count, 
                QUEUE_MAX_SIZE);

        /* Signal to wake printer up */
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&q_lock);
    }
    return NULL;
}

void* printer(void *arg)
{
    (void)arg;
    Document doc;
    while(1) {
        pthread_mutex_lock(&q_lock);
        while (count == 0 && !all_sent) {
            pthread_cond_wait(&not_empty, &q_lock);
        }

        if(dequeue(&doc) != 0){
            printf("Error: Queue full - waiting...\n");
            pthread_mutex_unlock(&q_lock);
        }
        doc_printed++;
        total_pages += doc.pages;
        printf("[Printer] Printing: %s, (%d pages) - queue: %d/%d\n", 
                doc.filename,
                doc.pages,
                count,
                QUEUE_MAX_SIZE);
        if (count == 0 && all_sent){
            printf("[Printer]    All documents printed. Exiting.\n");
            pthread_mutex_unlock(&q_lock);
            break;
        }
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&q_lock);
        sleep(SLEEP_DELAY);
    }
    return NULL;
}

void print_summary()
{
    const char *summary = 
"================ SUMMARY ================\n\
    Documents submitted : %d\n\
    Documents printed   : %d\n\
    Total pages printed : %d\n\
=========================================";
    printf(summary, doc_submitted, doc_printed, total_pages);

}

bool isFull() { return (tail == QUEUE_MAX_SIZE); }
bool isEmpty() { return (head == tail - 1); }

int enqueue(Document doc)
{
    queue[tail] = doc;
    tail = (tail + 1)%QUEUE_MAX_SIZE;
    count++;
    return 0;
}

int dequeue(Document *doc)
{
    *doc = queue[head];
    head = (head + 1)%QUEUE_MAX_SIZE;
    count--;
    return 0;
}
