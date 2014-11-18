#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

mqd_t qdes;
int total, numProducers, numConsumers;
sem_t * mayProduce; // named semaphore for the producer
sem_t * mayConsume; // named semaphore for the consumer
sem_t * valuesRemaining; // named semaphore for the tems remaining to be consumed

void produce(int id) {
    // open the semaphores for producer and consumer
    mayConsume = sem_open("/semc_a24joshi", 0);
    mayProduce = sem_open("/semp_a24joshi", 0);

    int i;
    // produce items in steps of number of producers
    for (i = id; i < total; i += numProducers) {
        // decrememnt the producer semaphore
        sem_wait(mayProduce);
        // try to add the item to the queue
        if (mq_send(qdes, (char *)&i, sizeof(int), 0) == -1) {
            perror("mq_send() failed");
        }
        // increment the consumer semaphore
        sem_post(mayConsume);
    }

    // close the two semaphores and exit the producer
    sem_close(mayConsume);
    sem_close(mayProduce);
    exit(0);
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

    // create the named consumer semaphore
    mayConsume = sem_open("/semc_a24joshi", O_CREAT, mode, 0);
    total = atoi(argv[1]);
    // create the named producer semaphore
    mayProduce = sem_open("/semp_a24joshi", O_CREAT, mode, atoi(argv[2]));
    // create the semaphore for values remaining with the total number of items as the initial value
    valuesRemaining = sem_open("/semt_a24joshi", O_CREAT, mode, total);
    numProducers = atoi(argv[3]);
    numConsumers = atoi(argv[4]);

    int producerId = numProducers, consumerId = numConsumers;
    pid_t children[numProducers + numConsumers]; //array of the children pids


    // if the process is a child, then replace the current process image with a new one
    while (consumerId--) {
        // create a child process using fork
        pid_t child_pid = fork();
        if (child_pid == 0) {
            // convert the consumer id to string to pass to execl
            char idString[20];
            sprintf(idString, "%d", consumerId);
            // execute consume for the given consumer by passing it's id
            execl("./consume", "consume", &idString, (char *)NULL);
        } else {
            // if parent, then keep track of it in the children array
            children[consumerId + numProducers] = child_pid;
        }
    }

    while (producerId--) {
        // create a child process using fork
        pid_t child_pid = fork();
        if (child_pid == 0) {
            produce(producerId);
        } else {
            // if parent, then keep track of it in the children array
            children[producerId] = child_pid;
        }
    }

    // wait for a state change in the child process
    int i;
    for (i = 0; i < numConsumers + numProducers; i++) {
        int status;
        // loop untill the child status has changed
        while (waitpid(children[i], &status, 0) == -1) {}
        // if status hasn't changed to exited, then incrememt i and wait for it to change to exited
        if (!WIFEXITED(status)) {
            i--;
        }
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

    // close all the semaphores
    sem_close(mayConsume);
    sem_close(mayProduce);
    sem_close(valuesRemaining);

    // remove all the semaphores
    sem_unlink("/semp_a24joshi");
    sem_unlink("/semc_a24joshi");
    sem_unlink("/semt_a24joshi");

    gettimeofday(&complete, &tz);

    // print the initialization and data transmission times
    printf("System execution time: %f seconds\n", computeTime(&complete, &start));

    return 0;
}

