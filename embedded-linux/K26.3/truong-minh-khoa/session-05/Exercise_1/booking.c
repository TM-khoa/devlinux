#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define THREAD_NUM 5
#define TOTAL_SEATS 10
#define SLEEP_DELAY 1

typedef struct {
    int  agent_id;
    char customer[50];
    int  seats_wanted;
} BookingRequest;

BookingRequest requests[5] = {
    {1, "Nguyen Van An",  2},
    {2, "Tran Thi Bich",  1},
    {3, "Le Van Cuong",   3},
    {4, "Pham Thi Dung",  4},
    {5, "Hoang Van Em",   2}
};

int seats_available = TOTAL_SEATS;
pthread_mutex_t seat_lock;
int seat_sold = 0;
int fail_booking = 0;

void* book_ticket(void *arg);
void print_summary();

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main() {
    printf("==============================================\n");
    printf("   TICKET BOOKING SYSTEM (5 agents, 10 seats) \n");
    printf("==============================================\n");

    pthread_mutex_init(&seat_lock, NULL);
    pthread_t thread[5];
    int result;
    for(int i = 0; i < THREAD_NUM;i++) {
        result = pthread_create(&thread[i], NULL, book_ticket, (void*)&requests[i]);
        if (result != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }
    printf("--- [all agents reach critical section after sleep(1)] ---\n");

    for(int i = 0; i < THREAD_NUM;i++) {
        int err = pthread_join(thread[i], NULL);
        if(err != 0) {
            printf("Error: Fail to join the thread\n");
        }
    }

    print_summary();
    pthread_mutex_destroy(&seat_lock);
    printf("Thread finished. Exiting main.\n");
    return 0;
}

void* book_ticket(void *arg)
{
    BookingRequest *request = NULL;
    if(arg != NULL) {
        request = (BookingRequest*)arg;
    }
    int wanted = request->seats_wanted;
    char *customer = request->customer;
    int id = request->agent_id;

    printf("[Agent %d | TID %lu...] Booking %d seats for %s...\n", id, (long)pthread_self(), wanted, customer);
    sleep(SLEEP_DELAY);
    pthread_mutex_lock(&seat_lock);
    /* Due to seats_available is a shared resource, the check condition and then deduct seats must be placed in the mutex lock, or else the race condition between threads will corrupt the value of seats_available */
    if(seats_available >= wanted) {
        seats_available -= wanted;
        seat_sold = TOTAL_SEATS - seats_available;
        printf("[Agent %d] CONFIRM: %d seats for %s. Remaining:%d\n", id,  wanted, customer, seats_available);
    }
    else {
        printf("[Agent %d] SOLD OUT: need %d seats only %d left - booking failed\n", id,  wanted, seats_available);
        fail_booking++;
    }

    pthread_mutex_unlock(&seat_lock);
    return NULL;

}

void print_summary()
{
    printf("================ SUMMARY ================\n");
    printf("  Total seats     : %d\n", TOTAL_SEATS);
    printf("  Seats sold      : %d\n", seat_sold);
    printf("  Seats remaining : %d\n", TOTAL_SEATS - seat_sold);
    printf("  Failed bookings : %d\n", fail_booking);
    printf("=========================================\n");

}
