#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#define WRITER_THREAD 10
#define READER_THREAD 500

#define READER_REPEAT 60
#define WRITER_REPEAT 30

static sem_t rw_mutex; //used by both reader and writer threads, mutual exclusion for writers
static sem_t  mutex; //ensure mutual exclusion when read_count is updated
static int read_count = 0; //how many threads are reading the object
static int Shared = 0;

int readers;
int writers;
void *thread_function (void *arg) {
	char *msg = arg;
	printf("%s\n", msg);
	return NULL;
}


void *writer(void *arg) { 
	
	
	int number_attempt = *((int *) arg);
	int attempt = 0;
	//	printf("attemptwriter: %d\n", number_attempt);

	while (attempt < number_attempt) {
		sem_wait(&rw_mutex);

		Shared+= 10;
		printf("val: %d\n", Shared);
		fflush(stdout);
		
		sem_post(&rw_mutex);
		usleep(1000);
		attempt++;
	}



}

void *reader(void *arg) { //if continuous flow of readers, writers will starve
	
	int number_attempt = *((int *) arg);
	int attempt = 0;
	//printf("attempt: %d\n", number_attempt);
	while (attempt < number_attempt) {
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
		usleep(1000);
		attempt++;
	} 








}


int main (int argc, char **argv) {
	
//	int g = 10;
//	printf("%s%s\n", argv[1], argv[2]);	
	int writer_repeat = atoi(argv[1]);
	int reader_repeat = atoi(argv[2]);
	sem_init(&mutex, 0,1);
	sem_init(&rw_mutex,0,1);

	pthread_t *thread_writer = malloc(sizeof(pthread_t) * WRITER_THREAD);
	pthread_t *thread_reader = malloc(sizeof(pthread_t) * READER_THREAD);

	int s;
	for (int i = 0; i < WRITER_THREAD; ++i) {
		pthread_create(&thread_writer[i], NULL, writer, &writer_repeat);	
	}
	
	for (int y = 0; y < READER_THREAD; ++y) {
		pthread_create(&thread_reader[y], NULL, reader, &reader_repeat);	
	}
	

	


	for (int i = 0; i < WRITER_THREAD; ++i) {
		pthread_join(thread_writer[i], NULL);
	}
	
	for (int i = 0; i < READER_THREAD; ++i) {
		pthread_join(thread_reader[i], NULL);
	}
	

	
	sem_destroy(&mutex);
	sem_destroy(&rw_mutex);	
	return EXIT_SUCCESS;	





}

