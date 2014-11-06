#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <time.h>

mqd_t qdes;
sem_t * sem_c; // named semaphore for the consumer

int consume(int expect) {
    while (expect--) {
        int item;
        // try to remove the oldest item in the queue
        if (mq_receive(qdes, (char *)&item, sizeof(int), NULL) == -1) {
            perror("mq_receive() failed");
        }
        printf("Consumed integer %d\n", item);
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

    // initialize the consumer semaphore with 0 as the initial value
    sem_c = sem_open("/semc_a24joshi", 0);

    // consume the items added into the queue by the producer
    consume(atoi(argv[1]));

    // try to close the queue
    if (mq_close(qdes) == -1) {
        perror("mq_close() failed");
        exit(2);
    }

    return 0;
}
