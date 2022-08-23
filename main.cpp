#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include "types_p2.h"
#include "p2_threads.h"
#include "utils.h"
#include <algorithm> 

#define THREADS  2

pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct timeval t_global_start;
struct timeval t_current;
int users;
std::vector<int> order;
std::vector<Person> people;



int main(int argc, char** argv)
{	
	gettimeofday(&t_global_start, NULL);

	// (10%) Command line output matches with the description for any type of input
	if (argc != 2) {
		printf("[ERROR] Expecting 1 argument, but got (%d). \n", argc);
		printf("[USAGE] p2_exec <number> \n");
		return 0;
	}
	//
	

	///// (15%) Randomly generate the input sequence and randomly assign the time to stay in the restroom 
	srand(time(NULL));
	users = atoi(argv[1]);	//GET USER INPUT
	//printf(" NUM: %d \n", users);
	//Randomly generate the order
	
	for (int ff = 0; ff < users * 2; ff++) {
		order.push_back(ff); 	
	}
	std::random_shuffle(order.begin(), order.end());
	int stay;
	Person p;
	for (int ff = 0; ff < users * 2; ff++) {
		if (order[ff] % 2 == 0) {
			//printf("male \n");
			p.set_gender(0);
		}
		else {
			//printf("female \n");
			p.set_gender(1);
		}	
		p.set_order(ff);
		p.set_use_order(-1);
		stay = rand() % (5 - 1 + 1) + 1;
		p.set_time(stay);
		people.push_back(p);
	}



	/////END OF SECTION





	// This is to set the global start time
	gettimeofday(&t_global_start, NULL);


	pthread_t       *tid = (pthread_t*)malloc(sizeof(pthread_t) * THREADS); //2 threads needed aside from main
	int             status = 0;
	int             work = 0;



	for (int i = 0; i < THREADS; ++i) {
		if (pthread_create(&tid[i], NULL, threadfunc, NULL)) {
			fprintf(stderr, "Error creating thread\n");
		}
	}
	//usleep(MSEC(10));
	


	//for (int i=0; i<5; i++) {
	//	printf("Wake up thread after (%d) seconds\n", (5-i));
	//	usleep(MSEC(1000));
	//}

	//printf("Wake up thread\n");
	//status = pthread_cond_signal(&cond);
	

	/* wait for the second thread to finish */

	//for (i = 0; i < 2; ++i) {
		//pthread_join(tid[i], NULL);
	//}

	for (int i = 0; i < THREADS; ++i) {
		if (pthread_join(tid[i], NULL)) {
			fprintf(stderr, "Error joining thread\n");
		}
	}




	return 0;


}

