#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

mqd_t qdes; // the message queue to send and receive numbers
pthread_mutex_t buffer; // mutex for the message queue
pthread_mutex_t numConsumedMutex; // mutex for the consumers
sem_t mayProduce, mayConsume; // unnamed semaphores
int numConsumed;
int numProducers, numConsumers, total;

// Used by the consumers to consume a given item
void* consume(void *id_pointer) {
    int id = * (int *)id_pointer;
    while (numConsumed < total) {
        // Check if there are items in the queue to be consumed
        if (sem_trywait(&mayConsume) == -1) {
            continue;
        }
        pthread_mutex_lock(&buffer);
        int item;
        // try to consume the given item from the message queue
        if (mq_receive(qdes, (char *)&item, sizeof(int), NULL) == -1) {
            perror("mq_receive() failed");
            pthread_mutex_unlock(&buffer);
            break;
        }
        // if successful in consuming an item
        pthread_mutex_lock(&numConsumedMutex);
        numConsumed++;
        pthread_mutex_unlock(&numConsumedMutex);
        pthread_mutex_unlock(&buffer);
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
        pthread_mutex_lock(&buffer);
        // try to add items to the message queue
        if (mq_send(qdes, (char *)&i, sizeof(int), 0) == -1) {
            perror("mq_produce() failed");
        }
        pthread_mutex_unlock(&buffer);
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
    
    char *qname = "/a24joshi_mqueue";
    int oflag = O_RDWR | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR;
    struct mq_attr attr;
    
    // check for valid number of arguments
    if ( argc !=5 ) {
        printf("Usage: %s <N> <B> <P> <C>\n", argv[0]);
        exit(1);
    }
    
    
    total = atoi(argv[1]);
    // initialize the semaphore with number of producers as the initial value
    sem_init(&mayProduce, 0, atoi(argv[2]));
    numProducers = atoi(argv[3]);
    numConsumers = atoi(argv[4]);
    // initialize the semaphore with zero as the initial value
    sem_init(&mayConsume, 0, 0);
    
    // Set the queue attributes
    attr.mq_maxmsg  = atoi(argv[2]);
    attr.mq_msgsize = sizeof(int);
    attr.mq_flags   = 0;            /* a blocking queue  */
    
    // create the queue with the given queue name and attributes
    qdes  = mq_open(qname, oflag, mode, &attr);
    
    // check if creating the queue was successful
    if (qdes == -1 ) {
        perror("mq_open() failed");
        exit(1);
    }
    
    // an array of threads of size equal to total number of producers and consumers
    pthread_t threads[numProducers + numConsumers];
    // arrays of producer and consumer ids
    int producerIds[numProducers];
    int consumerIds[numConsumers];
    int i;
    
    numConsumed = 0;
    pthread_mutex_init(&buffer, NULL);
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
    
    // try to close the queue
    if (mq_close(qdes) == -1) {
        perror("mq_close() failed");
        exit(2);
    }
    
    // try to remove the queue
    if (mq_unlink(qname) != 0) {
        perror("mq_unlink() failed");
        exit(3);
    }
    
    gettimeofday(&complete, &tz);
    
    // print the initialization and data transmission times
    printf("System execution time: %f seconds\n", computeTime(&complete, &start));
    
    return 0;
}

