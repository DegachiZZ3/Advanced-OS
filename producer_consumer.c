#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5  // Define buffer size

// Buffer structure
int buffer[BUFFER_SIZE];
int count = 0;      // Number of items in the buffer
int in = 0;         // Index for the next item to be produced
int out = 0;        // Index for the next item to be consumed

// Mutex and condition variables
pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

// Function to produce an item
void* producer(void* arg) {
    int item;
    for (int i = 0; i < 10; i++) { // Produce 10 items for this example
        item = rand() % 100; // Random item to produce
        pthread_mutex_lock(&mutex);

        // Wait if the buffer is full
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }

        // Add item to the buffer
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        printf("Produced: %d\n", item);

        // Signal that the buffer is not empty
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        sleep(1); // Simulate some production time
    }
    return NULL;
}

// Function to consume an item
void* consumer(void* arg) {
    int item;
    for (int i = 0; i < 10; i++) { // Consume 10 items for this example
        pthread_mutex_lock(&mutex);

        // Wait if the buffer is empty
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        // Remove item from the buffer
        item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("Consumed: %d\n", item);

        // Signal that the buffer is not full
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        sleep(1); // Simulate some consumption time
    }
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;

    // Initialize mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    // Create producer and consumer threads
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    // Wait for threads to finish
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    // Destroy mutex and condition variables
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return 0;
}
