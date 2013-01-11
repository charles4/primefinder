
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>


#define MAXDATASIZE 1000; //max number of bytes we can receive, arbitrary right now
#define PORT 8000; // port the server uses

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
	//networking information
	int status, sock, len;
	struct addrinfo hints;
	struct addrinfo *res;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	//get connection info
	status = getaddrinfo("127.0.0.1", "8000", &hints, &res);
	if(status != 0)
		fprintf(stderr, "error getting address info. \n");


	//local count is so that we're sure the number isn't going to be changed by some other thread in memory
	//and we only have to lock the global var to read it into the local one. 
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

			char buffer[100] = {0};
			char *eos = &buffer[0];
			eos += sprintf(buffer,"i %lu", localcount);
			//printf("sending command: %s \n", buffer);

			//create socket	
			sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if(sock == -1)
				fprintf(stderr, "error creating socket. \n");

			//connect to server
			status = connect(sock, res->ai_addr, res->ai_addrlen);
			if(status == -1)
				fprintf(stderr, "problem connecting to server.\n");
	
			//send number to server
			len = sizeof(localcount);
			status = send(sock, &buffer, len, 0);

			close(sock);						
		}

	}


	printf("end of thread\n");

	return NULL;
}

int requestNums(global_counter *globalcounter){

	global_counter *gc = globalcounter;

	//networking information
	int status, sock, len;
	struct addrinfo hints;
	struct addrinfo *res;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	//get connection info
	status = getaddrinfo("127.0.0.1", "8000", &hints, &res);
	if(status != 0)
		fprintf(stderr, "error getting address info. \n");

	//query server for more numbers to crunch
	//create socket	
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sock == -1)
		fprintf(stderr, "error creating socket. \n");

	//connect to server
	status = connect(sock, res->ai_addr, res->ai_addrlen);
	if(status == -1)
		fprintf(stderr, "problem connecting to server.\n");

	//send request for more numbers
	char * str = "n";	
	len = sizeof(str);
	status = send(sock, str, len, 0);

	//listen for response
	char buffer[100]; 
	status = recv(sock, buffer, sizeof buffer, 0);
	printf("received: %s\n", buffer);

	//close socket
	close(sock);	

	//check if response is the stopcode	
	char * stop = "stop";
	status = strncmp(stop, buffer, 4);
	printf("strncmp status = %d\n", status);
	
	if(status == 0)
		return 1;

	//check if response is wait
	status = strncmp("wait", buffer, 4);
	if(status == 0){
		printf("waiting... \n");
		return 0;
	}
	
	/*note to self: STRTOK IS NOT THREAD SAFE*/	
	//if status is not 0, then server returned a numset
	char * lownum = strtok(buffer, "-");
	char * highnum = strtok(NULL, "-");

	//convert to long ints
	long low = strtol(lownum, NULL, 10);
	long high = strtol(highnum, NULL, 10);

	printf("low: %lu\n", low);
	printf("high: %lu\n", high);

//	status = pthread_mutex_lock(&gc->mutex);
//	assert(0==status);

	gc->count = low;
	gc->maxcount = high;		
	
//	status = pthread_mutex_unlock(&gc->mutex);
//	assert(0==status);


	return 0;
}

int main (int argc, char *argv[]) {
	
	//setup four threads running thread_routine

	global_counter *gcounter;
	int status, stopcode;
	pthread_t thread1, thread2, thread3, thread4;

	//allocate meomry for our global counter struct
	gcounter = malloc(sizeof(global_counter));
	if(gcounter == NULL){
		printf("gcounter was null");
		return 1;
	}

	//initialize mutex
	status = pthread_mutex_init (&gcounter->mutex, NULL);	
	if(status != 0){
		assert(status==0);
	}

	//handle threads in loop
	//whenever threads finish, query server for more numbers or stop
	stopcode = 0;

	while(stopcode != 1){

		//get number range from server
		//requestNums updates the global struct
	  	stopcode = requestNums(gcounter);

		//launch threads
		status = pthread_create (&thread1, NULL, thread_routine, (void*)gcounter);
		if(status !=0){
			assert(status==0);	
		}
		
		status = pthread_create (&thread2, NULL, thread_routine, (void*)gcounter);
		if(status !=0){
			assert(status==0);
		}
	
		status = pthread_create (&thread3, NULL, thread_routine, (void*)gcounter);
		if(status !=0){
			assert(status==0);
		}

		status = pthread_create (&thread4, NULL, thread_routine, (void*)gcounter);
		if(status !=0){
			assert(status==0);
		}

	


		status = pthread_join (thread1, NULL);
		assert(0==status);
	
		status = pthread_join (thread2, NULL);
		assert(0==status);

		status = pthread_join (thread3, NULL);
		assert(0==status);
		
		status = pthread_join (thread4, NULL);
		assert(0==status);


	}
	
	//free memory from global counter struct
	(void)free (gcounter);

	printf("program exiting now.\n");
	return 0;
}
