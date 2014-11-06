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
sem_t sem_p; // unnamed semaphore for the producer
sem_t * sem_c; //named semaphore for the consumer

int produce(int expect) {
	// Seed the RNG with a random-ish value.
    srand(time(0));
    
    while (expect--) {
		int random = rand();
        // try to add a random integer to the queue
        if (mq_send(qdes, (char *)&random, sizeof(int), 0) == -1) {
            perror("mq_send() failed");
        }
        printf("Produced integer %d\n", random);
        // increment the consumer semaphore
		sem_post(sem_c);
	}
    
	return 0;
}

// Calculates the initialization and the setup time for the process
double computeTime(struct timeval *bench, struct timeval *ref) {
	int secDiff = bench->tv_sec - ref->tv_sec;
	int usecDiff = bench->tv_usec - ref->tv_usec;
	return (secDiff*1000000 + usecDiff)/1000000.0;
}

int main(int argc, char *argv[]) {
	struct timeval start, setup, complete;
	struct timezone tz;
	gettimeofday(&start, &tz);
    
    char *qname = "/a24joshi_mqueue";
	int oflag = O_RDWR | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR;
    struct mq_attr attr;
    
    // check for valid number of arguments
    if ( argc !=3 ) {
        printf("Usage: %s <N> <B>\n", argv[0]);
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
    
    // initialize the unnamed producer semaphore
	sem_init(&sem_p, 0, atoi(argv[1]));
    
    // create the named consumer semaphore
	sem_c = sem_open("/semc_a24joshi", O_CREAT, mode, 0);
    
    // create a child process using fork
	pid_t child_pid = fork();
    
    // if the process is a child, then replace the current process image with a new one
	if (child_pid == 0) {
		execl("./consume", "consume", argv[1], (char *)NULL);
	}
    
	gettimeofday(&setup, &tz);
    
    // produce the number of random items asked by the user
	produce(atoi(argv[1]));
    
    // try to close the queue
    if (mq_close(qdes) == -1) {
        perror("mq_close() failed");
        exit(2);
    }
	
    // wait for a state change in the child process
	int status;
	wait(&status);
    
    // try to remove the queu
    if (mq_unlink(qname) != 0) {
        perror("mq_unlink() failed");
        exit(3);
    }
    
	gettimeofday(&complete, &tz);
    
    // print the initialization and data transmission times
	printf("Time to initialize system: %f seconds\n", computeTime(&setup, &start));
	printf("Time to transmit data: %f seconds\n", computeTime(&complete, &start));
    
	return 0;
}

