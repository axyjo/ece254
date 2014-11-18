#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

mqd_t qdes;
pthread_mutex_t buffer;
pthread_mutex_t numConsumedMutex;
sem_t mayProduce, mayConsume;
int numConsumed;
int numProducers, numConsumers, total;

void* consume(void *id_pointer) {
    int id = * (int *)id_pointer;
    while (numConsumed < total) {
        if (sem_trywait(&mayConsume) == -1) {
            continue;
        }
	pthread_mutex_lock(&buffer);
        int item;
        if (mq_receive(qdes, (char *)&item, sizeof(int), NULL) == -1) {
            perror("mq_receive() failed");
            pthread_mutex_unlock(&buffer);
            break;
        }

        pthread_mutex_lock(&numConsumedMutex);
        numConsumed++;
        pthread_mutex_unlock(&numConsumedMutex);
	pthread_mutex_unlock(&buffer);
        sem_post(&mayProduce);

        double root = sqrt(item);
        if (floor(root) == root) {
	    printf("%d %d %d\n", id, item, (int)root);
        }
    }

    return NULL;
}

void* produce(void *id_pointer) {
    int id = * (int *)id_pointer;
    int i;

    for (i = id; i < total; i = i + numProducers) {
        sem_wait(&mayProduce);
	pthread_mutex_lock(&buffer);
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
    sem_init(&mayProduce, 0, atoi(argv[2]));
    numProducers = atoi(argv[3]);
    numConsumers = atoi(argv[4]);

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

    pthread_t threads[numProducers + numConsumers];
    int producerIds[numProducers];
    int consumerIds[numConsumers];
    int i;

    numConsumed = 0;
    pthread_mutex_init(&buffer, NULL);
    pthread_mutex_init(&numConsumedMutex, NULL);

    for (i = 0; i < numProducers; i++) {
        producerIds[i] = i;
        pthread_create(&threads[i], NULL, produce, &producerIds[i]);
    }

    for (i = 0; i < numConsumers; i++) {
        consumerIds[i] = i;
        pthread_create(&threads[numProducers + i], NULL, consume, &consumerIds[i]);
    }

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

