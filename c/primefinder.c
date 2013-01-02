
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>



typedef struct global_counter {
	pthread_mutex_t mutex;
	long	maxcount;
	long 	count;
} global_counter;

int check_if_prime(long anumber){
	//a prime number is only divisible by itself and 1
	
	int i = 3;
	//conditions
	if(anumber == 1)
		return 1;
	if(anumber == 2)
		return 1;
	if(anumber%2 == 0)
		return 0;

	while(anumber%i != 0){
		i += 2;
	}

	if(anumber == i)
		return 1;
	else
		return 0;

}

void *thread_routine(void *arg){
	//local count is so that we're sure the number isn't going to be changed by some other thread in memory
	//and we only have to lock the global var to read it into the local one. 
	int status;
	long localcount;
	global_counter *gc = ((global_counter*)arg);

	printf("opening thread\n");	

	while (gc->count < gc->maxcount){
		status = pthread_mutex_lock(&gc->mutex);
		assert(0==status);
			
		if (gc->count < gc->maxcount){
			localcount=gc->count;
			gc->count++;
		}else{
			return NULL;
		}
	
		status = pthread_mutex_unlock(&gc->mutex);
		assert(0==status);

		//now call prime checker on localcount
		if(check_if_prime(localcount) == 1){
			printf("%lu, ", localcount);
		}
	}

	printf("end of thread\n");

	return NULL;
}

int main (int argc, char *argv[]) {
	

	//setup four threads running thread_routine

	global_counter *gcounter;
	int status;
	char *end;
	pthread_t thread1;

	//allocate meomry for our global counter struct
	gcounter = malloc(sizeof(global_counter));
	if(gcounter == NULL){
		printf("gcounter was null");
		return 1;
	}

	//interpret command line values
	if (argc != 2){
		printf("usage: %s searchceiling\n\n", argv[0]);
		return 0;
	}else{
		gcounter->maxcount = strtol(argv[1], &end, 10);
		//printf("gcount->maxcount = %lu \n", gcounter->maxcount);
	}

	//initialize remaining global counter struct values
	gcounter->count = 1;

	status = pthread_mutex_init (&gcounter->mutex, NULL);	
	if(status != 0){
		assert(status==0);
	}

	//launch threads
	status = pthread_create (&thread1, NULL, thread_routine, (void*)gcounter);
	if(status !=0){
		assert(status==0);	
	}
	
	status = pthread_join (thread1, NULL);
	assert(0==status);
	
	//free memory from global counter struct
	(void)free (gcounter);

	printf("program exiting now.\n");
	return 0;
}
