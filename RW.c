#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>






static sem_t rw_mutex = 1; //used by both reader and writer threads, mutual exclusion for writers
static sem_t mutex = 1; //ensure mutual exclusion when read_count is updated
static int read_count = 0; //how many threads are reading the object


void *thread_function (void *arg) {
	char *msg = arg;
	printf("%s\n", msg);
	return NULL;
}


void *writer(void *arg) { 
	do {
		sem_wait(&rw_mutex);

		printf("hello, %d\n", wait);
		
		sem_post(&rw_mutex);


	} while(true);




}

void *reader(void *arg) { //if continuous flow of readers, writers will starve

	do {
		sem_wait(&mutex);
		read_count++;
		if (read_count == 1) {
			sem_wait(&rw_mutex);
		}
		sem_post(&mutex);

		printf("writers, %d\n", wait);
		
		sem_wait(&mutex);
		read_count--;
		if (read_count == 0) {
			sem_post(&rw_mutex);
		}
		sem_post(&mutex);
	} while(true);












}


int main (void) {

	int wait;
	int post;


		
	/*
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
	*/

}

