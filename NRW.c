#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#define WRITER_THREAD 10
#define READER_THREAD 500

#define MICRO 1E9

typedef enum boolean {false, true} bool;
static sem_t rw_mutex; //used by both reader and writer threads, mutual exclusion for writers
static sem_t  mutex; //ensure mutual exclusion when read_count is updated
static sem_t fair;
int ctrin = 0;
int ctrout = 0;
bool wait = false;



 
static int read_count = 0; //how many threads are reading the object
static int Shared = 0; //shared integer edited in critical section 

//variables to get statistics 
double MINREADTIME = 1; 
double MAXREADTIME = 1;
double MINWRITETIME = 1;
double MAXWRITETIME = 1;
double AVERAGEREAD = 0;
double AVERAGEWRITE = 0;

int readers;
int writers;


int RNG() { //generates random number between 0 and 100 for sleep
	int RandomSleep = rand() % (101);
	return RandomSleep * 1000;	
}

void *writer(void *arg) { 
	
	
	int number_attempt = *((int *) arg);
	int attempt = 0;

	while (attempt < number_attempt) { //loop number of times specified in cmd argument

		struct timespec tstart={0,0}, tend={0,0}; //initialize timer
		clock_gettime(CLOCK_MONOTONIC, &tstart); //start timer
	
		sem_wait(&mutex);
		sem_wait(&rw_mutex);

		clock_gettime(CLOCK_MONOTONIC, &tend); //end timer

		double microsecond = (MICRO * (tend.tv_sec - tstart.tv_sec) + tend.tv_nsec - tstart.tv_nsec)/1000; //calculate time in microseconds
		AVERAGEWRITE += microsecond; //add to average


		if (microsecond < MINWRITETIME) { //adjust min
			MINWRITETIME = microsecond;
		}
		if (microsecond > MAXWRITETIME) { //adjust max
			MAXWRITETIME = microsecond;
		}

		if (ctrin == ctrout) {
			sem_post(&rw_mutex);
		}
		else {
			wait = true;
			sem_post(&rw_mutex);
			sem_wait(&fair);
			wait = false;
	
		}
		//start critical section
		Shared+= 10;
		//end critical section		

		
		 //sleep thread using RNG function
		sem_post(&mutex);
		attempt++;
		int sleep = RNG();
		usleep(sleep);
		
	}



}

void *reader(void *arg) { //if continuous flow of readers, writers will starve
	
	int number_attempt = *((int *) arg);
	int attempt = 0;
	//printf("attempt: %d\n", number_attempt);
	while (attempt < number_attempt) {
		struct timespec tstart={0,0}, tend={0,0}; //initialize timer 
		clock_gettime(CLOCK_MONOTONIC, &tstart); //start timer
		
		sem_wait(&mutex);

		clock_gettime(CLOCK_MONOTONIC, &tend);
		
		double microsecond = (MICRO * (tend.tv_sec - tstart.tv_sec) + tend.tv_nsec - tstart.tv_nsec)/1000; //calculate time in microseconds
		AVERAGEREAD += microsecond; //add to average


		if (microsecond < MINREADTIME) { //adjust min
			MINREADTIME = microsecond;
		}
		if (microsecond > MAXREADTIME) { //adjust max
			MAXREADTIME = microsecond;
		}

		ctrin++;


		sem_post(&mutex);


		//start critical section 
		printf("read: %d\n", Shared);
		//end critical section		


		sem_wait(&rw_mutex);
		ctrout++;
		

		if (wait == true && ctrin==ctrout) {
			sem_post(&fair);

		}
		sem_post(&rw_mutex);
	
		int sleep = RNG();
		usleep(sleep);
		attempt++;
	} 








}


int main (int argc, char **argv) {
	
//	int g = 10;
//	printf("%s%s\n", argv[1], argv[2]);	
	int writer_repeat = atoi(argv[1]);
	int reader_repeat = atoi(argv[2]);
	sem_init(&mutex, 0,1); //initialize semaphores
	sem_init(&rw_mutex,0,1);
	sem_init(&fair, 0,0);

	//initialize threads
	pthread_t *thread_writer = malloc(sizeof(pthread_t) * WRITER_THREAD);
	pthread_t *thread_reader = malloc(sizeof(pthread_t) * READER_THREAD);

	//create threads
	for (int i = 0; i < WRITER_THREAD; ++i) {
		pthread_create(&thread_writer[i], NULL, writer, &writer_repeat);	
	}
	
	for (int y = 0; y < READER_THREAD; ++y) {
		pthread_create(&thread_reader[y], NULL, reader, &reader_repeat);	
	}
	

	

	//join threads
	for (int i = 0; i < WRITER_THREAD; ++i) {
		pthread_join(thread_writer[i], NULL);
	}
	
	for (int i = 0; i < READER_THREAD; ++i) {
		pthread_join(thread_reader[i], NULL);
	}
	

	
	sem_destroy(&mutex);
	sem_destroy(&rw_mutex);	


	//print statistics
	printf("min read: %f\n", MINREADTIME);
	printf("max read: %f\n", MAXREADTIME);
	printf("min write: %f\n", MINWRITETIME);
	printf("max write: %f\n", MAXWRITETIME);
	printf("average read: %f\n", AVERAGEREAD/(READER_THREAD*reader_repeat) );
	printf("average write: %f\n", AVERAGEWRITE/(WRITER_THREAD*writer_repeat) );



	return EXIT_SUCCESS;	





}

