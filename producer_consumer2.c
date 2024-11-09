#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

// Buffer size and number of producers/consumers
#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define NUM_ITEMS 10

// Semaphore identifiers
enum {MUTEX, EMPTY, FULL};

// Shared buffer structure
struct shared_buffer {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    int count;
};

// Semaphore operations
void semaphore_wait(int sem_id, int sem_num) {
    struct sembuf operation = {sem_num, -1, 0};
    semop(sem_id, &operation, 1);
}

void semaphore_signal(int sem_id, int sem_num) {
    struct sembuf operation = {sem_num, 1, 0};
    semop(sem_id, &operation, 1);
}

// Create semaphores: mutex, empty, and full
int create_semaphore_set() {
    int sem_id = semget(IPC_PRIVATE, 3, 0666 | IPC_CREAT);
    if (sem_id == -1) {
        perror("Failed to create semaphores");
        exit(1);
    }

    // Initialize semaphores
    semctl(sem_id, MUTEX, SETVAL, 1);         // Mutex initialized to 1
    semctl(sem_id, EMPTY, SETVAL, BUFFER_SIZE); // Empty slots initialized to BUFFER_SIZE
    semctl(sem_id, FULL, SETVAL, 0);           // Full slots initialized to 0

    return sem_id;
}

// Create shared memory for the buffer
int create_shared_memory() {
    int shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_buffer), 0666 | IPC_CREAT);
    if (shm_id == -1) {
        perror("Failed to create shared memory");
        exit(1);
    }
    return shm_id;
}

// Producer process function
void producer(int shm_id, int sem_id, int producer_id) {
    struct shared_buffer *buf = (struct shared_buffer *)shmat(shm_id, NULL, 0);
    if (buf == (void *)-1) {
        perror("Producer: Failed to attach shared memory");
        exit(1);
    }

    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = rand() % 100; // Produce a random item

        semaphore_wait(sem_id, EMPTY); // Wait if buffer is full
        semaphore_wait(sem_id, MUTEX); // Lock buffer access

        // Critical Section: Add item to the buffer
        buf->buffer[buf->in] = item;
        buf->in = (buf->in + 1) % BUFFER_SIZE;
        buf->count++;
        printf("Producer %d: Produced %d\n", producer_id, item);

        semaphore_signal(sem_id, MUTEX); // Unlock buffer access
        semaphore_signal(sem_id, FULL);  // Signal that buffer is not empty

        sleep(1); // Simulate production time
    }
    shmdt(buf); // Detach shared memory
}

// Consumer process function
void consumer(int shm_id, int sem_id, int consumer_id) {
    struct shared_buffer *buf = (struct shared_buffer *)shmat(shm_id, NULL, 0);
    if (buf == (void *)-1) {
        perror("Consumer: Failed to attach shared memory");
        exit(1);
    }

    for (int i = 0; i < NUM_ITEMS; i++) {
        semaphore_wait(sem_id, FULL);  // Wait if buffer is empty
        semaphore_wait(sem_id, MUTEX); // Lock buffer access

        // Critical Section: Remove item from the buffer
        int item = buf->buffer[buf->out];
        buf->out = (buf->out + 1) % BUFFER_SIZE;
        buf->count--;
        printf("Consumer %d: Consumed %d\n", consumer_id, item);

        semaphore_signal(sem_id, MUTEX); // Unlock buffer access
        semaphore_signal(sem_id, EMPTY); // Signal that buffer has space

        sleep(1); // Simulate consumption time
    }
    shmdt(buf); // Detach shared memory
}

int main() {
    srand(time(NULL)); // Seed for random number generation

    // Create shared memory and semaphores
    int shm_id = create_shared_memory();
    int sem_id = create_semaphore_set();

    // Initialize shared buffer
    struct shared_buffer *buf = (struct shared_buffer *)shmat(shm_id, NULL, 0);
    buf->in = 0;
    buf->out = 0;
    buf->count = 0;

    // Create producer processes
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        if (fork() == 0) {
            producer(shm_id, sem_id, i + 1);
            exit(0);
        }
    }

    // Create consumer processes
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        if (fork() == 0) {
            consumer(shm_id, sem_id, i + 1);
            exit(0);
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_PRODUCERS + NUM_CONSUMERS; i++) {
        wait(NULL);
    }

    // Clean up shared memory and semaphores
    shmdt(buf);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    return 0;
}
