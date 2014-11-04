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
sem_t sem_p;
sem_t * sem_c;

int produce(int expect) {
	// Seed the RNG with a random-ish value.
        srand(time(0));

        while (expect--) {
        //while (sem_trywait(&sem_p) == 0) {
		int random = rand();
                if (mq_send(qdes, (char *)&random, sizeof(int), 0) == -1) {
                        perror("mq_send() failed");
                }
                printf("Produced integer %d\n", random);
		sem_post(sem_c);
	}

	return 0;
}

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

        if ( argc !=3 ) {
                printf("Usage: %s <N> <B>\n", argv[0]);
                exit(1);
        }

        attr.mq_maxmsg  = atoi(argv[2]);
        attr.mq_msgsize = sizeof(int);
        attr.mq_flags   = 0;            /* a blocking queue  */

        qdes  = mq_open(qname, oflag, mode, &attr);
        if (qdes == -1 ) {
                perror("mq_open() failed");
                exit(1);
        }

	sem_init(&sem_p, 0, atoi(argv[1]));
	sem_c = sem_open("/semc_a24joshi", O_CREAT, mode, 0);

	pid_t child_pid = fork();
	if (child_pid == 0) {
		execl("./consume", "consume", argv[1], (char *)NULL);
	}
	gettimeofday(&setup, &tz);

	produce(atoi(argv[1]));

        if (mq_close(qdes) == -1) {
                perror("mq_close() failed");
                exit(2);
        }
	
	int status;
	wait(&status);

        if (mq_unlink(qname) != 0) {
                perror("mq_unlink() failed");
                exit(3);
        }
	gettimeofday(&complete, &tz);

	printf("Time to initialize system: %f seconds\n", computeTime(&setup, &start));
	printf("Time to transmit data: %f seconds\n", computeTime(&complete, &start));

	return 0;
}

