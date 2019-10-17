#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>


void *thread_function (void *arg) {
	char *msg = arg;
	printf("%s\n", msg);
	return NULL;
}

int main (void) {
	pthread_t t1, t2;
	int s;
	char *msg = "hello world";



	s = pthread_create(&t1, NULL, thread_function, msg);
	s |= pthread_create(&t2, NULL, thread_function, msg);

	if (s != 0) exit(1);

	s |= pthread_join(t1, NULL);
	s |= pthread_join(t2, NULL);


	if (s != 0) exit(1);
	
	return 0;

}

