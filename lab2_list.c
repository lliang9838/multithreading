#include "SortedList.h" //for the 4 functions that was implemented

#include <getopt.h> //for getopt long options
#include <pthread.h> //pthread library, remember to compile with -pthread
#include <stdlib.h>
#include <stdio.h>/* for printf */
#include <stdlib.h>/* for exit() definition */
#include <time.h>/* for clock_gettime */
#include <string.h>


//Options that can be passed in
#define threadOption 't'
#define iterationOption 'i'
#define yieldOption 'y'
#define syncOption 's'

char sync = '\0'; //sync character is a nullbyte
int threadNum = 1; //default value for thread #
int iterationNum = 1; //default value for iteration #
int listNum = 1; //only one list is allowed

/* SORTED_LIST AND SORTEDLISTELEMENT DECLARATIONS */
SortedList_t list; //head, will need to pass in as &head, unlike kar
SortedListElement_t *elementArray; //our element array
int elementArrayNum = 0; //number of elements in the array


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //LOCK INITIALIZED


void * threadFunc( void * arg);

#define secs2nano 1000000000L //the L makes it a long

//SPIN-LOCK IMPLEMENTATION, FROM LAB2_ADD.C
typedef struct lock_t
{
  int flag;
} lock_t;


lock_t testLock; //the test Lock, will use testLock


void init(lock_t *lock)
{
  lock->flag = 0; //need to init before beginning
}

void spin_lock(lock_t *lock) //just put these guys next to critical section
{
  while( __sync_lock_test_and_set(&lock->flag,1)==1)
      ; //spin-wait(do nothing) waste CPU cycles, page 310 of arpaci
}
 void spin_unlock(lock_t *lock)
 {
  //lock->flag = 0;
    __sync_lock_release(&lock->flag);
 }


int opt_yield = 0;

char randomChar()
{
	return (char)((rand() % 78) + 30);
}

char * randomKey() //remember to cite the website that u sent to abdullah
{
	//function used to generate random keys
	 //9 characters and last one is nullbyte
	char *key = malloc(15 * sizeof(char)); //hint, length of key is 15
	int i;

	srand(time(NULL)); //choice of the seed is continually changing

	for ( i = 0; i < 14; i++)
	{
		key[i] = randomChar();
	}

	key[14] = '\0'; //last element in the string is terminated by the null byte
	return key;
}



int main(int argc, char*argv[]) //basic command line interface
{

	init(&testLock); //LOCK INITIALIZER

	static struct option long_options[]={

		{"threads", required_argument, 0, threadOption}, //named threads
		{"iterations", required_argument, 0, iterationOption},
		{"yield", required_argument, 0, yieldOption},
		{"sync", required_argument, 0, syncOption},
		{0, 0, 0, 0}

	};


	int ret = 0;
	while(1)
	{
		size_t length; //cannot put declaration in switch statements, lols
		//used size_t so no warnings would occur
		ret = getopt_long(argc, argv,"", long_options,0);
		if ( ret == -1)
			break;

		switch(ret)
		{
			case threadOption:
				threadNum = atoi(optarg);
				break;
			case iterationOption:
				iterationNum = atoi(optarg);
				break;
			case yieldOption:
				length = strlen(optarg); //since yield can have multiple combinations
				if ( length > 3)
				{
					fprintf(stderr, "Wrong number of arguments for yield option!\n");
					exit(1);
				}
				int i;
				for ( i=0; i < length; i++)
				{
					switch(optarg[i])
					{
						case 'i':
							opt_yield = opt_yield | INSERT_YIELD; // |=  is equal to +=
							break;
						case 'd':
							opt_yield = opt_yield | DELETE_YIELD; // |= is equal to +=
							break;
						case 'l':
							opt_yield = opt_yield | LOOKUP_YIELD; // |= is equal to +=
							break;
					}

				}
				break;
			case syncOption:
				sync = optarg[0]; //sync holds chars, just set first ele in optarg to it
				if ( sync != 's' && sync != 'm')
				{
					fprintf(stderr, "Only mutex or spinlocks allowed as sync methods!\n"); //I can do sync
					exit(1);
				} //no need to init mutex since its already initialized
				break; 
			default:
				fprintf(stderr, "Usage is incorrect. Please try again.\n");
				exit(1);
		}

	}

	//it's time to initialize

	SortedList_t *header = &list;
	list.next = header;
	list.prev = header;
	list.key = NULL; //key for the head is null

	elementArrayNum = threadNum * iterationNum; //number of list elements

	elementArray = malloc( elementArrayNum * sizeof(SortedListElement_t)); 
	//element array has arraynum of elements, let dynamically create with malloc

	//initializing the elements in my array with a random key
	int i;
	for ( i = 0; i < elementArrayNum; i++)
	{
		elementArray[i].key = randomKey();
	}

	//now I need to record start time
	struct timespec startTime;
	clock_gettime(CLOCK_MONOTONIC, &startTime);

	//creating my threads and passing them into the thread func
	pthread_t * tid = malloc( threadNum * sizeof(pthread_t));
	if ( tid == NULL)
	{
		fprintf(stderr, "Error with malloc during pthread_t!\n");
		
		exit(1);
	}

	int *t_value = (int*) malloc(threadNum * sizeof(int));
	/* CREATING */
	//threading threadNum threads with pthread_create

	//printf("Before create!\n");

	int j;
	for ( j = 0; j < threadNum; j++)
	{

		t_value[j] = j;
		//printf("Inside create, my j is %d\n", j);

		if ( pthread_create(&tid[j],NULL, threadFunc, &t_value[j]) != 0 )
		{
			fprintf(stderr, "Error: cannot create thread!s\n"); //remember to cite his work
			
			exit(1);
		}
	}

	int k;

	/* WAIT FOR ALL THREADS TO COMPLETE */

	// printf("Before join!\n");
	// printf("Thread number is %d!\n", threadNum);

	for ( k = 0; k < threadNum; k++)
	{
		//printf("Inside join %d!\n", k);

		if (pthread_join(tid[k], NULL) != 0)
		{
			fprintf(stderr, "Error: cannot join threads!\n");
			
			exit(1);
		}

		//printf("Joined thread number: %d!\n", k);
	}

	//printf("After join!\n");
	/* RECORD END TIME */
	//after joining, time to record end time

	struct timespec endTime;
	clock_gettime(CLOCK_MONOTONIC, &endTime);


	/* CHECKS THE LENGTH OF THE IST TO CONFIRM THAT IT IS ZERO */

	int ec = 0;
	int listlength = SortedList_length(&list);
  	if ( listlength != 0 )
  	{
    	fprintf(stderr, "Error: the length of the list is not zero!\n");
    	ec = 1;
  	}



	long long operationNum = threadNum * iterationNum * 3; //3 operations bc del, insert, lookup
	long long diff = secs2nano * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_nsec - startTime.tv_nsec;

	long long avgtime = diff / operationNum;

	
	/* WILL PRINT SIMILARLY TO WHAT I DID FOR LAB2_ADD.C */
	printf("list");

	printf("-"); //so there is a few hyphen

	//check the different yield options
	if ( opt_yield & INSERT_YIELD) //these should cover up all the situations
	{
		printf("i");
	}
	if ( opt_yield & DELETE_YIELD)
	{
		printf("d");
	}
	if ( opt_yield & LOOKUP_YIELD)
	{
		printf("l");
	}
	if ( opt_yield == 0)
	{
		printf("none");
	}


	//now it's time for synchronization options
	if ( sync == '\0') //compare to the nullbytem if not modified,-none
	{
		printf("-none");
	}
	else if ( sync == 'm') //mutex
	{
		printf("-m");
	}
	else //now we are left with spinlock
	{
		printf("-s");
	}


	//format to print: name of test, threadNum, iterationNum, list num, total # operat, total run time, avg time per operation

	printf(",%d,%d,%d,%lld,%lld,%lld\n",threadNum, iterationNum,listNum,operationNum,diff,avgtime);


	exit(ec); //zero means sucessful

}

void * threadFunc( void * arg)
{

	//let's do mutex flag first
	if ( sync == 'm')
	{
		//let's lock with mutex
		
		pthread_mutex_lock(&lock);
	}
	if ( sync == 's')
	{
		//let's lock with spin lock
   	    spin_lock(&testLock);
	}


	int threadData = *((int*)arg); //data contains the thread data

	int from = (threadData * elementArrayNum)/threadNum; //from this amt
	int to = ( (threadData+1) * elementArrayNum)/threadNum; //to that amt


	// printf("\n");
	// printf("threadData is: %d!\n", threadData);

	// printf("from is: %d!\n", from);
	// printf("to is: %d!\n", to);
	// printf("elementArrayNum is: %d!\n", elementArrayNum);
	// printf("\n");

	//INSERTING
	int i;
	for ( i = from; i< to; i++)
	{	
		//printf("i is %d!\n",i);

		//printf("\n");
		//printf("INSIDE INSERTION!\n");
		SortedList_insert(&list, &elementArray[i]);
	}

	//GETTING LENGTH
	int ret = 0;
	ret = SortedList_length(&list); //just following spec
	if ( ret == -1 )
	{
		fprintf(stderr, "Error: SortedList is corrupted after insertion!\n");
		exit(2);
	}


	//LOOKS UP AND DELETES EACH OF THE KEYS IT PREVIOUSLY INSERTED

	int j;
	for ( j = from; j < to; j++)
	{
		// printf("look loop, j iteration is: %d!\n",j);

		// printf("\n");
		// printf("BEFORE LOOKUP!\n");
		SortedListElement_t * current = SortedList_lookup(&list, elementArray[j].key);
		if ( current == NULL)
		{
			fprintf(stderr, "Error: SortedListed_lookup in list is corrupted!\n");
			exit(1);
		}

		//need to check for delete corruption now
		// printf("\n");
		// printf("BEFORE DELETE!\n");
		if ( SortedList_delete(current) != 0 )
		{
			fprintf(stderr, "Error: SortedList_delete in list is corrupted!\n");
			exit(1);
		}
		// printf("\n");
		// printf("AFTER DELETE!\n");

	}

	if ( sync == 'm')
	{
		pthread_mutex_unlock(&lock); //release the mutex
	}

	if ( sync == 's')
	{
		spin_unlock(&testLock); //release teslock
	}


	return NULL;
}









