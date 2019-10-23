#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#define WRITER_THREAD 10
#define READER_THREAD 500

#define MICRO 1E3


static sem_t rw_mutex; //used by both reader and writer threads, mutual exclusion for writers
static sem_t  mutex; //ensure mutual exclusion when read_count is updated
static sem_t fair; //ensures ordering of requests
static int read_count = 0; //how many threads are reading the object
static int Shared = 0;

double MINREADTIME = 1;
double MAXREADTIME = 1;
double MINWRITETIME = 1;
double MAXWRITETIME = 1;

double AVERAGEREAD = 0;
double AVERAGEWRITE = 0;
int readers;
int writers;

void *thread_function (void *arg) {
	char *msg = arg;
	printf("%s\n", msg);
	return NULL;
}

int RNG() {
	int RandomSleep = rand() % (101);
	return RandomSleep * 1000;	


}

void *writer(void *arg) { 
	
	
	int number_attempt = *((int *) arg);
	int attempt = 0;
	//	printf("attemptwriter: %d\n", number_attempt);

	while (attempt < number_attempt) {

		sem_wait(&fair);

		struct timespec tstart={0,0}, tend={0,0};
		clock_gettime(CLOCK_MONOTONIC, &tstart);
	
		sem_wait(&rw_mutex);

		clock_gettime(CLOCK_MONOTONIC, &tend);

		double microsecond = (tend.tv_sec - tstart.tv_sec )+
			       (tend.tv_nsec - tstart.tv_nsec)
				 / MICRO;
		AVERAGEWRITE += microsecond;


		if (microsecond < MINWRITETIME) {
			MINWRITETIME = microsecond;
		}
		if (microsecond > MAXWRITETIME) {
			MAXWRITETIME = microsecond;
		}
		

		sem_post(&fair);
		//start critical section
		Shared+= 10;
		printf("val: %d\n", Shared);
		fflush(stdout);
		//end critical section		

		sem_post(&rw_mutex);

		usleep(RNG());
		attempt++;
	}



}

void *reader(void *arg) { //if continuous flow of readers, writers will starve
	
	int number_attempt = *((int *) arg);
	int attempt = 0;
	//printf("attempt: %d\n", number_attempt);
	while (attempt < number_attempt) {

		sem_wait(&fair);


		struct timespec tstart={0,0}, tend={0,0};
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		
		sem_wait(&mutex);

		clock_gettime(CLOCK_MONOTONIC, &tend);

		double microsecond = (tend.tv_sec - tstart.tv_sec )+
			       (tend.tv_nsec - tstart.tv_nsec)
				 / MICRO;

		AVERAGEREAD += microsecond;
		if (microsecond < MINREADTIME) {
			MINREADTIME = microsecond;
		}
		if (microsecond > MAXREADTIME) {
			MAXREADTIME = microsecond;
		}

		read_count++;

		if (read_count == 1) {
			sem_wait(&rw_mutex);
		}

		sem_post(&fair);
		sem_post(&mutex);

		//start critical section 
		printf("read: %d\n", Shared);
		fflush(stdout);	
		//end critical section		

		sem_wait(&mutex);
		read_count--;
		if (read_count == 0) {
			sem_post(&rw_mutex);
		}
		sem_post(&mutex);
		usleep(RNG());
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
	sem_init(&fair,0,1);

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

	printf("min read: %f\n", MINREADTIME);
	printf("max read: %f\n", MAXREADTIME);
	printf("min write: %f\n", MINWRITETIME);
	printf("max write: %f\n", MAXWRITETIME);
	printf("average read: %f\n", AVERAGEREAD/(READER_THREAD*reader_repeat) );
	printf("average write: %f\n", AVERAGEWRITE/(WRITER_THREAD*writer_repeat) );



	return EXIT_SUCCESS;	





}

