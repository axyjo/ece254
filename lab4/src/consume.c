#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <time.h>

mqd_t qdes;
sem_t * sem_c;

int consume(int expect) {
	//while (sem_wait(sem_c) == 0) {
	while (expect--) {
		int item;
		if (mq_receive(qdes, (char *)&item, sizeof(int), NULL) == -1) {
			perror("mq_receive() failed");
		}
		printf("Consumed integer %d\n", item);
	}

	return 0;
}

int main(int argc, char *argv[]) {
        char *qname = "/a24joshi_mqueue";
        qdes  = mq_open(qname, O_RDWR);

        if (qdes == -1 ) {
                perror("mq_open() failed");
                exit(1);
        }

	sem_c = sem_open("/semc_a24joshi", 0);

	consume(atoi(argv[1]));

        if (mq_close(qdes) == -1) {
                perror("mq_close() failed");
                exit(2);
        }

	return 0;
}
