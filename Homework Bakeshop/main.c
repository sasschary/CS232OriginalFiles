#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <sched.h>

#define NUM_CUSTOMERS 10
#define MAX_IN_STORE  5
#define SECOND * 1000000


/* --------------------------------------------- FUNCTION DECLARATIONS ---------------------------------------------- */

void createSemaphores();
void createThreads();

void* doCustomeryThings(void* arg);
void* doBakeryThings();
void* doSellyThings();

/* ------------------------------------------------ GLOBAL VARIABLES ------------------------------------------------ */

sem_t g_customersInStore;
sem_t g_customersInLine;
sem_t g_loavesAvailable;
sem_t g_bakerIsAvailable;

sem_t g_firstPersonHasLoaf;
sem_t g_firstPersonHasPaid;
sem_t g_firstPersonHasReceipt;

pthread_t g_bakerBakeThread;
pthread_t g_bakerSellThread;
pthread_t g_customers[NUM_CUSTOMERS];

/* -------------------------------------------- FUNCTION IMPLEMENTATIONS -------------------------------------------- */

int main() {
    fprintf(stderr, "The COVID-19-less Bakery is now open for business!\n");
    createSemaphores();
    createThreads();
    pthread_exit(NULL);
}

void createSemaphores() {
    sem_init(&g_customersInStore, 0, MAX_IN_STORE);
    sem_init(&g_customersInLine, 0, MAX_IN_STORE);
    sem_init(&g_loavesAvailable, 0, 0);
    sem_init(&g_bakerIsAvailable, 0, 1);
    sem_init(&g_firstPersonHasLoaf, 0, 0);
    sem_init(&g_firstPersonHasPaid, 0, 0);
    sem_init(&g_firstPersonHasReceipt, 0, 0);
}

void createThreads() {
    pthread_create(&g_bakerBakeThread, NULL, &doBakeryThings, NULL);
    pthread_create(&g_bakerSellThread, NULL, &doSellyThings, NULL);

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_create(&(g_customers[i]), NULL, &doCustomeryThings, i);
    }
}

void* doCustomeryThings(void* arg) {
    int id = (int)arg;

    // Put ourself into the queue of people
    sem_wait(&g_customersInStore);
    sem_wait(&g_customersInLine);
    fprintf(stderr, "Customer %i: \"Hi, Mr. Baker!\"\n", id);

    // Wait for the baker to give us a loaf
    sem_wait(&g_firstPersonHasLoaf);
    usleep(1 SECOND);
    fprintf(stderr, "Customer %i: \"Thanks for the loaf!\"\n", id);
    usleep(1 SECOND);

    // Pay the baker
    sem_post(&g_firstPersonHasPaid);
    fprintf(stderr, "Customer %i: \"Here's your money.\"\n", id);

    // Take your receipt
    sem_wait(&g_firstPersonHasReceipt);
    fprintf(stderr, "Customer %i: \"Thank you, kind sir.\"\n", id);

    // Leave the store.
    usleep(1 SECOND);
    fprintf(stderr, "Customer %i: \"Bye now!\"\n", id);
    usleep(1 SECOND);
    sem_post(&g_customersInStore);
}

void* doBakeryThings() {

    int num;
    for (int i = 0; i < 10; i++) {
        // Make sure I'm available
        sem_wait(&g_bakerIsAvailable);

        // Make a loaf of bread, then make myself available again
        usleep(1 SECOND);
        sem_post(&g_loavesAvailable);
        fprintf(stderr, "Baker: \"Now that looks like a good loaf of bread!\"\n");
        sem_post(&g_bakerIsAvailable);
    }

    fprintf(stderr, "Baker: \"I think that's enough bread for one day.\"\n");
}

void* doSellyThings() {
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        // Wait for a customer to join the line
        sem_post(&g_customersInLine);

        // Make sure I'm available
        sem_wait(&g_bakerIsAvailable);

        // Take a loaf and give it to the customer
        sem_wait(&g_loavesAvailable);
        sem_post(&g_firstPersonHasLoaf);
        fprintf(stderr, "Baker: \"Here's your loaf!\"\n");

        // Wait for the customer to pay
        sem_wait(&g_firstPersonHasPaid);
        usleep(1 SECOND);
        fprintf(stderr, "Baker: \"Thanks for not running off with the bread.\"\n");

        // Give the customer their receipt
        usleep(1 SECOND);
        sem_post(&g_firstPersonHasReceipt);
        fprintf(stderr, "Baker: \"Here's your receipt.\"\n");

        sem_post(&g_bakerIsAvailable);
    }
}