#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>

mqd_t qdes;
sem_t * mayConsume; // named semaphore for consumer
sem_t * mayProduce; // named semaphore for producer
sem_t * valuesRemaining; // named semaphore for values remaming

int consume(int id) {
    // while there are items in the message queue, try to consume them
    while (sem_trywait(valuesRemaining) == 0) {
        // decrease the consumer semaphore
        sem_wait(mayConsume);
        int item;
        // try to remove the oldest item in the queue
        if (mq_receive(qdes, (char *)&item, sizeof(int), NULL) == -1) {
            perror("mq_receive() failed");
        }

        // do some math the check if item is perfect square
        double root = sqrt(item);
        if (root == floor(root)) {
            printf("%d %d %d\n", id, item, (int)root);
        }
        // increase the producer semaphore
        sem_post(mayProduce);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    char *qname = "/a24joshi_mqueue";
    qdes  = mq_open(qname, O_RDWR); // open the queue with the given queue name

    // check if opening the queue was successful
    if (qdes == -1 ) {
        perror("mq_open() failed");
        exit(1);
    }

    // open the consumer, producer and values remaining semaphores
    mayConsume = sem_open("/semc_a24joshi", 0);
    mayProduce = sem_open("/semp_a24joshi", 0);
    valuesRemaining = sem_open("/semt_a24joshi", 0);

    // consume the items added into the queue by the producer
    consume(atoi(argv[1]));

    // try to close the queue
    if (mq_close(qdes) == -1) {
        perror("mq_close() failed");
        exit(2);
    }

    // close all the semaphores
    sem_close(mayConsume);
    sem_close(mayProduce);
    sem_close(valuesRemaining);

    exit(0);
    return 0;
}
