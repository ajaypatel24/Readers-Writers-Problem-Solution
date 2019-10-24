#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#define WRITER_THREAD 10
#define READER_THREAD 500

#define MICRO 1E9

typedef enum boolean {false, true} bool; //boolean type
static sem_t exit_sem; //exit semaphore
static sem_t  in; //entry semaphore 
static sem_t write_sem; //write semaphore
int ctrin = 0; //entry count readers
int ctrout = 0; //exit count readers
bool wait = false; //condition used by writer for safety



 
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
	
		if ( sem_wait(&in) == -1) //semaphore indiciating entering 
			exit(2);

		 
		if ( sem_wait(&exit_sem) == -1) //semphore indicating exiting
			exit(2); 

		clock_gettime(CLOCK_MONOTONIC, &tend); //end timer


		// timing block start
		double microsecond = (MICRO * (tend.tv_sec - tstart.tv_sec) + tend.tv_nsec - tstart.tv_nsec)/1000; //calculate time in microseconds
		AVERAGEWRITE += microsecond; //add to average


		if (microsecond < MINWRITETIME) { //adjust min
			MINWRITETIME = microsecond;
		}
		if (microsecond > MAXWRITETIME) { //adjust max
			MAXWRITETIME = microsecond;
		}
		// timing block end 


		if (ctrin == ctrout) { //number of readers in = number of readers out (no readers)
			if ( sem_post(&exit_sem) == -1 ) //unlock exiting semaphore
				exit(2); 	
		}
		else { //if there are still readers, lock them out and proceed to write
			wait = true; //part of condition in reader that releases write semaphore 
			if (sem_post(&exit_sem) == -1) //release exiting semaphore 
				exit(2); 
			if (sem_wait(&write_sem) == -1) //hold writing semaphore
				exit(2); 
			wait = false;
	
		}

		//start critical section
		Shared+= 10;
		//end critical section		

		
		//sleep thread using RNG function
		if (sem_post(&in) == -1)  //release input semaphore so readers can enter again
			exit(2);
		attempt++;
		int sleep = RNG();
		usleep(sleep);
		
	}



}

void *reader(void *arg) { //if continuous flow of readers, writers will starve
	
	int number_attempt = *((int *) arg);
	int attempt = 0;

	while (attempt < number_attempt) {
		struct timespec tstart={0,0}, tend={0,0}; //initialize timer 
		clock_gettime(CLOCK_MONOTONIC, &tstart); //start timer
		
		if ( sem_wait(&in) == -1 ) 
			exit(2); //reader takes entry semaphore which locks out writer

		clock_gettime(CLOCK_MONOTONIC, &tend);
		
		//start timing section 
		double microsecond = (MICRO * (tend.tv_sec - tstart.tv_sec) + tend.tv_nsec - tstart.tv_nsec)/1000; //calculate time in microseconds
		AVERAGEREAD += microsecond; //add to average


		if (microsecond < MINREADTIME) { //adjust min
			MINREADTIME = microsecond;
		}
		if (microsecond > MAXREADTIME) { //adjust max
			MAXREADTIME = microsecond;
		}
		//end timing section 


		ctrin++; //count reader threads that have entered


		if ( sem_post(&in) == -1) //release entry semaphore 
			exit(2);

		//start critical section 
		printf("read: %d\n", Shared);
		//end critical section		


		if ( sem_wait(&exit_sem) == -1) //claim exiting semaphore, writer requires both to perform any action
			exit(2); 
		ctrout++; //count reader threads that are exiting
		

		if (wait == true && ctrin==ctrout) { //writer sets wait to true and all reader threads have exited
			if ( sem_post(&write_sem) == -1) //release writing semaphore
				exit(2);
		}
		if ( sem_post(&exit_sem) == -1)
			exit(2); //release exit semaphore
	
		int sleep = RNG();
		usleep(sleep);
		attempt++;
	} 








}


int main (int argc, char **argv) {
	
	int writer_repeat = atoi(argv[1]);
	int reader_repeat = atoi(argv[2]);

	if (sem_init(&in, 0,1) == -1) { //initialize semaphores
		printf("Semaphore init error\n");
	}

	if (sem_init(&exit_sem,0,1) == -1) {
		printf("Semaphore init error\n");
	}

	if (sem_init(&write_sem, 0,0) == -1) {
		printf("Semaphore init error\n");
	}

	//initialize threads
	pthread_t *thread_writer = malloc(sizeof(pthread_t) * WRITER_THREAD);
	pthread_t *thread_reader = malloc(sizeof(pthread_t) * READER_THREAD);

	//create threads
	int s;
	for (int i = 0; i < WRITER_THREAD; ++i) {
		s = pthread_create(&thread_writer[i], NULL, writer, &writer_repeat);	
		if (s != 0) {
			printf("Thread creation error\n");
			exit(1);
		}
			
	}
	
	for (int y = 0; y < READER_THREAD; ++y) {
		s = pthread_create(&thread_reader[y], NULL, reader, &reader_repeat);	
		if (s != 0) {
			printf("Thread creation error\n");
			exit(1);
		}	
	}
	

	

	//join threads
	for (int i = 0; i < WRITER_THREAD; ++i) {
		s = pthread_join(thread_writer[i], NULL);
		if (s != 0) {
			printf("Thread join error\n");
		}
	}
	
	for (int i = 0; i < READER_THREAD; ++i) {
		s = pthread_join(thread_reader[i], NULL);
		if (s != 0) {
			printf("Thread join error\n");
		}
	}
	

	
	sem_destroy(&in);
	sem_destroy(&exit_sem);
	sem_destroy(&write_sem);	


	//print statistics
	int status = system("clear");
	printf("STATISTICS\n");
	printf("WRITES: %d, READS: %d\n", writer_repeat, reader_repeat); 
	printf("All units in Microseconds\n");
	printf("Min read: %fus\n", MINREADTIME);
	printf("Max read: %fus\n", MAXREADTIME);
	printf("Min write: %fus\n", MINWRITETIME);
	printf("Max write: %fus\n", MAXWRITETIME);
	printf("Mean read: %fus\n", AVERAGEREAD/(READER_THREAD*reader_repeat) );
	printf("Mean write: %fus\n", AVERAGEWRITE/(WRITER_THREAD*writer_repeat) );



	return EXIT_SUCCESS;	

}


