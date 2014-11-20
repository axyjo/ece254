#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t bufferMutex; // mutex for the message queue
pthread_mutex_t numConsumedMutex; // mutex for the consumers
sem_t mayProduce, mayConsume; // unnamed semaphores
int numConsumed;
int numProducers, numConsumers, total;
int* buffer;
int bufferPosition = 0;

// Used by the consumers to consume a given item
void* consume(void *id_pointer) {
    int id = * (int *)id_pointer;
    while (numConsumed < total) {
        // Check if there are items in the queue to be consumed
        if (sem_trywait(&mayConsume) == -1) {
            continue;
        }
        pthread_mutex_lock(&bufferMutex);
        int item = buffer[bufferPosition--];
        // if successful in consuming an item
        pthread_mutex_lock(&numConsumedMutex);
        numConsumed++;
        pthread_mutex_unlock(&numConsumedMutex);
        pthread_mutex_unlock(&bufferMutex);
        sem_post(&mayProduce);

        // do some math to check if item is perfect square
        double root = sqrt(item);
        if (floor(root) == root) {
            printf("%d %d %d\n", id, item, (int)root);
        }
    }

    return NULL;
}

// Used by the producers to produce the set of items
void* produce(void *id_pointer) {
    int id = * (int *)id_pointer;
    int i;
    // produce items in steps of the number of producers
    for (i = id; i < total; i = i + numProducers) {
        sem_wait(&mayProduce);
        pthread_mutex_lock(&bufferMutex);
        // try to add items to the message queue
        buffer[++bufferPosition] = i;
        pthread_mutex_unlock(&bufferMutex);
        sem_post(&mayConsume);
    }

    return NULL;
}

// Calculates the initialization and the setup time for the process
double computeTime(struct timeval *bench, struct timeval *ref) {
    int secDiff = bench->tv_sec - ref->tv_sec;
    int usecDiff = bench->tv_usec - ref->tv_usec;
    return (secDiff*1000000 + usecDiff)/1000000.0;
}

int main(int argc, char *argv[]) {
    struct timeval start, complete;
    struct timezone tz;
    gettimeofday(&start, &tz);

    // check for valid number of arguments
    if ( argc !=5 ) {
        printf("Usage: %s <N> <B> <P> <C>\n", argv[0]);
        exit(1);
    }


    total = atoi(argv[1]);
    // initialize the semaphore with number of producers as the initial value
    sem_init(&mayProduce, 0, atoi(argv[2]));
    buffer = calloc(atoi(argv[2]), sizeof(int));
    numProducers = atoi(argv[3]);
    numConsumers = atoi(argv[4]);
    // initialize the semaphore with zero as the initial value
    sem_init(&mayConsume, 0, 0);

    // an array of threads of size equal to total number of producers and consumers
    pthread_t threads[numProducers + numConsumers];
    // arrays of producer and consumer ids
    int producerIds[numProducers];
    int consumerIds[numConsumers];
    int i;

    numConsumed = 0;
    pthread_mutex_init(&bufferMutex, NULL);
    pthread_mutex_init(&numConsumedMutex, NULL);

    // Create threads for producers, calling produce for every producer while passing the id as a parameter
    for (i = 0; i < numProducers; i++) {
        producerIds[i] = i;
        pthread_create(&threads[i], NULL, produce, &producerIds[i]);
    }

    // Create threads for consumer, calling consume for every consumer while passing the id as a parameter
    for (i = 0; i < numConsumers; i++) {
        consumerIds[i] = i;
        pthread_create(&threads[numProducers + i], NULL, consume, &consumerIds[i]);
    }

    // Wait for all the threads to be done
    for (i = 0; i < numProducers + numConsumers; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&complete, &tz);

    free(buffer);

    // print the initialization and data transmission times
    printf("System execution time: %f seconds\n", computeTime(&complete, &start));


    return 0;
}

