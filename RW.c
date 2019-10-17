#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>

#define WRITER_THREAD 10
#define READER_THREAD 500



static sem_t rw_mutex; //used by both reader and writer threads, mutual exclusion for writers
static sem_t  mutex; //ensure mutual exclusion when read_count is updated
static int read_count = 0; //how many threads are reading the object
static int Shared = 0;

void *thread_function (void *arg) {
	char *msg = arg;
	printf("%s\n", msg);
	return NULL;
}


void *writer(void *arg) { 
	do {
		sem_wait(&rw_mutex);

		Shared+= 10;
		printf("val: %d\n", Shared);
		fflush(stdout);
		
		sem_post(&rw_mutex);

	} while(1);

	return NULL;



}

void *reader(void *arg) { //if continuous flow of readers, writers will starve

	do {
		sem_wait(&mutex);
		read_count++;
		if (read_count == 1) {
			sem_wait(&rw_mutex);
		}
		sem_post(&mutex);

		
		printf("read: %d\n", Shared);
		fflush(stdout);	
		
		sem_wait(&mutex);
		read_count--;
		if (read_count == 0) {
			sem_post(&rw_mutex);
		}
		sem_post(&mutex);
	} while(1);


	return NULL;









}


int main (void) {
	
	sem_init(&mutex, 0,1);
	sem_init(&rw_mutex,0,1);

	pthread_t *thread_writer = malloc(sizeof(pthread_t) * WRITER_THREAD);
	pthread_t *thread_reader = malloc(sizeof(pthread_t) * READER_THREAD);

	int s;
	for (int i = 0; i < WRITER_THREAD; ++i) {
		pthread_create(&thread_writer[i], NULL, writer, NULL);	
	}
	
	for (int y = 0; y < READER_THREAD; ++y) {
		pthread_create(&thread_reader[y], NULL, reader, NULL);	
	}
	

	


	for (int i = 0; i < WRITER_THREAD; ++i) {
		pthread_join(thread_writer[i], NULL);
	}
	
	for (int i = 0; i < READER_THREAD; ++i) {
		pthread_join(thread_reader[i], NULL);
	}
	
	
	return EXIT_SUCCESS;	





}

