#include "p2_threads.h"
#include "utils.h"
#include <queue>

extern pthread_cond_t  cond;
extern pthread_mutex_t mutex;

extern struct timeval t_global_start;
//extern struct timeval t_current;

extern int users;
extern std::vector<Person> people;

long long i = 0;

std::queue<Person> q;
	
//flags
int WomenPresent = 0;
int MenPresent = 0;
int Empty = 1;

int num_inside = 0;
int real_order = 1;
		
int gender_q;
int gender_s;

//GET TIME
int timer() {
	long ret = 0;
	struct timeval t_curr;
	gettimeofday(&t_curr, NULL);
	ret = get_elasped_time(t_global_start, t_curr);
	return ret;
}

//

///EXITS
void woman_leaves() {
	//printf("ay? \n");
	for (int z = 0; z < people.size();++z) {
		if (people[z].ready_to_leave() && people[z].get_gender() == 1 && MenPresent == 0 && WomenPresent == 1 && Empty == 0 && people[z].get_use_order() != -1) {
			pthread_mutex_lock(&mutex);

			num_inside--;
			if (num_inside > 0) {
				WomenPresent = 1;
				Empty = 0;
			}
			else {
				WomenPresent = 0;
				Empty = 1;
			}

			people[z].set_use_order(-1);

			printf("[%ld ms][Restroom] (Woman) left the restroom.Status is changed, Status is(-) : Total: %d(Men : %d, Women : %d)\n", timer(), (num_inside + q.size()), q.size(), num_inside);
			pthread_mutex_unlock(&mutex);
		}

	}

}

void man_leaves() {
	//printf("ay? \n");
	for (int z = 0; z < people.size();++z) {
		if (people[z].ready_to_leave() && people[z].get_gender() == 0 && MenPresent == 1 && WomenPresent == 0 && Empty == 0 && people[z].get_use_order() != -1) {
			pthread_mutex_lock(&mutex);

			num_inside--;
			if (num_inside > 0) {
				MenPresent = 1;
				Empty = 0;
			}
			else {
				MenPresent = 0;
				Empty = 1;
			}
			people[z].set_use_order(-1);

			//void Person::set_use_order(unsigned long data) {
			//	use_order = data;

			//people[z].ready_to_leave()
			printf("[%ld ms][Restroom] (Man) left the restroom.Status is changed, Status is(-) : Total: %d(Men : %d, Women : %d)\n", timer(), (num_inside+ q.size()), num_inside, q.size());
			// release lock	
			pthread_mutex_unlock(&mutex);
		}
	}
	
	
}
///END OF EXITS

///WANTS_TO_ENTER
void man_enters(int sel)
{
	// acquire lock
	if (WomenPresent == 1) {
		pthread_mutex_lock(&mutex);
		printf("[ERROR]	man should be queued up instead: Now Enqueueing \n");
		printf("[Input] A person(Man) goes into");
		q.push(people[sel]);
		printf(" the queue\n");
		pthread_mutex_unlock(&mutex);
		return;
	}
	num_inside++;
	MenPresent = 1;
	Empty = 0;

	// release lock	
	//printf("SEL: %d \n", sel);
	pthread_mutex_lock(&mutex);
	people[sel].start();
	pthread_mutex_unlock(&mutex);
	while (!(people[sel].ready_to_leave())) {
		usleep(MSEC(1)); // Wait 1 MS
	}


	



	//man_leaves(sel);


}


void woman_enters(int sel)
{
	// acquire lock
	if (MenPresent == 1) {
		pthread_mutex_lock(&mutex);
		printf("[ERROR]	woman should be queued up instead of sent in: Now Enqueueing");
		printf("[Input] A person(Woman) goes into");
		q.push(people[sel]);
		printf(" the queue\n");
		pthread_mutex_unlock(&mutex);
		return;
	}

	// release lock	
	pthread_mutex_lock(&mutex);
	people[sel].start();
	pthread_mutex_unlock(&mutex);
	while (!(people[sel].ready_to_leave())) {
		usleep(MSEC(1)); // Wait 1 MS
	}

}
///
void man_wants_to_enter(int sel)
{

	//When a person goes into the restroom, you must randomly assign the time to stay in the restroom (3 milliseconds – 10 milliseconds).
	//rest time so it follows instructions
	pthread_mutex_lock(&mutex);
	int stay = rand() % (5 - 1 + 1) + 1;
	people[sel].set_time(stay);
	pthread_mutex_unlock(&mutex);

	if (people[sel].get_gender() == 1) {
		printf("[Error] Woman is not man");
	}
	
	usleep(MSEC(1)); //give women high 
	if ((Empty == 1 || MenPresent == 1) && WomenPresent == 0)
	{

		printf("[%d ms][Restroom] Send(Man) into the restroom(Stay %d ms), Status: Total: %d(Men : %d, Women : %d)\n", timer(), stay, (num_inside)+q.size(), num_inside, q.size());

		man_enters(sel);
		
	}
	else
	{
		pthread_mutex_lock(&mutex);
		printf("[Input] A person(Man) goes into");
		q.push(people[sel]);
		printf(" the queue\n");
		pthread_mutex_unlock(&mutex);
	}
}


void woman_wants_to_enter(int sel)
{

	//When a person goes into the restroom, you must randomly assign the time to stay in the restroom (3 milliseconds – 10 milliseconds).
	//rest time so it follows instructions
	pthread_mutex_lock(&mutex);
	int stay = rand() % (5 - 1 + 1) + 1;
	people[sel].set_time(stay);
	pthread_mutex_unlock(&mutex);

	if (people[sel].get_gender() == 0) {
		printf("[Error] Man is not woman");
	}

	//printf("MenPresent: %d", MenPresent);
	if ((Empty == 1 || WomenPresent == 1) && MenPresent == 0)
	{
		pthread_mutex_lock(&mutex);
		num_inside++;
		WomenPresent = 1;
		Empty = 0;
		pthread_mutex_unlock(&mutex);

		printf("[%d ms][Restroom] Send(Woman) into the restroom(Stay %d ms), Status: Total: %d(Men : %d, Women : %d)\n", timer(), stay, (num_inside+ q.size()), q.size(), num_inside);

		

		woman_enters(sel);
		//printf("A woman (%d,%d) has entered the bathroom.\n", woman.id, woman.usage);
	}
	else
	{
		pthread_mutex_lock(&mutex);
		printf("[Input] A person(Woman) goes into");
		q.push(people[sel]);
		printf(" the queue\n");
		pthread_mutex_unlock(&mutex);
	}
}
///

///Enter
void enter_rest(int sel)

{
	if (people[sel].get_use_order() != -1) {
		return;
	}

	pthread_mutex_lock(&mutex);
	people[sel].set_use_order(sel);
	pthread_mutex_unlock(&mutex);

	if (q.size() > 0)
	{
		//pthread_mutex_lock(&mutex);
		Person qp = q.front();
		//pthread_mutex_unlock(&mutex);


		if (people[sel].get_gender() == 1 && (WomenPresent == 1 || Empty == 1 )&& qp.get_gender() == 1)
		{
			//printf("Empty: %d	MenPresent: %d	WomenPresent: %d ~~~W",Empty,MenPresent,WomenPresent);
			
			do { //Empty Queue into restroom
				//lock here
				if (q.empty()) {
					break;
				}
				
				Person out_q = qp;
				pthread_mutex_lock(&mutex);
				q.pop();
				num_inside++;
				WomenPresent = 1;
				Empty = 0;
				pthread_mutex_unlock(&mutex);
				int stay = rand() % (5 - 1 + 1) + 1;
				out_q.set_time(stay);
				printf("[%d ms][Queue] Send(Woman) into the restroom(Stay %d ms), Status: Total: %d(Men : %d, Women : %d)\n", timer(), stay, (num_inside+ q.size()), q.size(), num_inside);
				

				woman_enters((int)out_q.get_use_order());

				qp = q.front();

			} while (q.size() > 0 && qp.get_gender() == 1 && people[sel].get_gender() == 1);

		}
		else if (people[sel].get_gender() == 0 &&( MenPresent == 1 || Empty == 1) && qp.get_gender() == 0)
		{
			//printf("Empty: %d	MenPresent: %d	WomenPresent: %d ~~~M", Empty, MenPresent, WomenPresent);
			

			do {
				if (q.empty()) {
					break;
				}
				//lock here
				pthread_mutex_lock(&mutex);
				Person out_q = qp;
				q.pop();
				int stay = rand() % (5 - 1 + 1) + 1;
				out_q.set_time(stay);
				pthread_mutex_unlock(&mutex);
				//

				printf("[%d ms][Queue] Send(Man) into the restroom(Stay %d ms), Status: Total: %d(Men : %d, Women : %d)\n",timer(),stay, (num_inside+ q.size()), num_inside, q.size());
				man_enters((int)out_q.get_use_order());

				pthread_mutex_lock(&mutex);
				qp = q.front();
				pthread_mutex_unlock(&mutex);
	//			firstGender = get_first_in_queue_gender();
			} while (q.size() > 0 && qp.get_gender() == 0 && people[sel].get_gender() == 0);
		}
		else {
			//printf("\n Sel Gender: %d	Q Gender: %d WomanPresnet: %d ManPresent: %d Empty: %d \n", people[sel].get_gender(), qp.get_gender(), WomenPresent, MenPresent, Empty);
		}

	
	}

	
	if (people[sel].get_gender() == 0) {
		man_wants_to_enter(sel);
	}
	else {	
		woman_wants_to_enter(sel);
	}
	
}





void exit_rest()
{
	if (Empty == 0 && num_inside > 0) {
		if (WomenPresent == 1 && MenPresent == 0) {
			woman_leaves();
		}
		else if (MenPresent == 1 && WomenPresent == 0) {
			man_leaves();
		}
	}

}



///End of Exit


void *threadfunc(void *parm)
{

	int status;

	
	printf("[Thread] Start\n");
	int zzz;

	for (int x = 0;x<people.size();++x) {
		
		//exit_rest();
		enter_rest(x);
		exit_rest();
		//printf("EMPTY: %d	MenPresent: %d	WomenPresent: %d	Q: %d	SEL: %d \n",Empty,MenPresent,WomenPresent, q.size(), x);
	
		//printf("X: %d \n", x);

		zzz = x;
	}

	if (q.size() > 0 && zzz == people.size()){
		printf("[~~Only queue remains, time to empty it~~] \n");

	}
	///


	printf("[Thread] Complete\n");
	//if (q.size() > 0) {
	//	Person dddd = q.front();
	//	gender_q = dddd.get_gender();
	//	gender_s = dddd.size();
	//}
	

	
}


///////////////////////////
//Shared stuff


// acquire lock
//status = pthread_mutex_lock(&mutex);
//usleep(MSEC(10));
//printf("Person: %d", person.get_order())
// release lock	
//	status = pthread_mutex_unlock(&mutex);


//	printf(" [Thread] Locks\n");  //critical section
//	status = pthread_mutex_lock(&mutex);
//Operations here
//	for(i = 0;i <100;i++) {
//		printf('%d', i);
//	}

//printf(" [Thread] Unlocks\n");
//status = pthread_mutex_unlock(&mutex);


//
//  printf(" [Thread] Blocked\n");
//  status = pthread_cond_wait(&cond, &mutex);

//printf(" [Thread] Starts again.\n");
//for (int i=0; i<3; i++) {
//	printf(" [Thread] Complete thread after (%d) seconds\n", (3-i));
//	usleep(MSEC(1000));
//}




// Example code for sleep and class usage
//Person p1;
//p1.set_order(1);

//usleep(MSEC(200));
//p1.start();


//usleep(MSEC(150));
//p1.complete();
///////////////////////////////////////////


