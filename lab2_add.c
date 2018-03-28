//#define _GNU_SOURCE
#include <getopt.h> //for getopt long options
#include <pthread.h> //pthread library, remember to compile with -pthread
#include <stdlib.h>
#include <stdio.h>/* for printf */
#include <stdlib.h>/* for exit() definition */
#include <time.h>/* for clock_gettime */
#include <string.h>

#define threadOption 't' //option for thread
#define iterationOption 'i' //option for iterationOption
#define yieldOption 'y'
#define syncOption 's'

/* CHARACTERS FOR SYNCHRONIZATION */
char sync = '\0';
int mutexFlag = 0;
int spinFlag = 0;
int casFlag = 0; //compare and swap
int syncFlag = 0;

int exclusion = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //LOCK INITIALIZED

int threadNum = 1; //default # of threads is 1
int iterationNum = 1; //default # of iterations is 1
int opt_yield = 0;


#define secs2nano 1000000000L //the L makes it a long


//long long counter = 0; //intializes counter to 1
int spinlock = 0;



typedef struct lock_t
{
  int flag;
} lock_t;

lock_t test1lock; //lock for spin
lock_t test2lock; //lock for cas

void init(lock_t *lock)
{
  lock->flag = 0;
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


void cas_lock(lock_t *lock)
{
  while(__sync_val_compare_and_swap(&lock->flag, 0, 1) == 1)
    ; //spin, do nothing, page 312 of arpaci
}

void cas_unlock(lock_t *lock)
{
  lock->flag = 1;
}

//basic add routine that I have to start with
void add(long long *pointer, long long value)
{

  long long sum = *pointer + value;
  if (opt_yield) //to more easily cause failures  
      sched_yield();
     
  *pointer = sum;

}

// void add_compare(long long *pointer, long long value)
// {
//   long long prev;
//   long long sum;
//   do
//   {
//     prev = *pointer;
//     sum = prev + value;
//     if(opt_yield)
//       sched_yield();
//   } while(__sync_val_compare_and_swap(pointer, prev, sum) != prev);
// }

// void improved_add(int val) //my better add that takes care of locks and such
// {
  
//   else if ( syncFlag && mutexFlag) //mutex flag is on
//   {
//     pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //LOCK INITIALIZED

//     pthread_mutex_lock(&lock);
//     add(&counter, val);
//     pthread_mutex_unlock(&lock);
//   }
//   else if ( syncFlag && spinFlag)
//   {
//     init(&test1lock);
//     spin_lock(&test1lock);
//     add(&counter, val);
//     spin_unlock(&test1lock);
//   }
//   else //syncFlag and compare and swap
//   {
//     // init(&test2lock);
//     // cas_lock(&test2lock);
//     // add(&counter,val);
//     // cas_unlock(&test2lock);
    
//   //  add_compare((long long*)counter, val);

//     fprintf(stderr, "Before long long new old.\n");

//     long long new, old;
//     do{

//       fprintf(stderr, "Inside do-while. Before assignment.\n");

//       old = counter;
//       new = old + val;
//         if (opt_yield)
//         {
//           sched_yield();
//         }

//       fprintf(stderr, "Inside do-while. After assignment.\n");
//     }while(__sync_val_compare_and_swap((long long *)counter, old, new) !=  old);

//     fprintf(stderr, "Outside do-while loop.\n");
//   }

// }



//threadAdd is the function pointer that I'm passing in
void *threadAdd(void *counter) 
{
  int i = 0;
  int numTimes = 0; //number of times the function is gonna call add
  //numTimes = *((int*)arg);

  //adds 1 to counter
  while( i < iterationNum) //add already has opt_yield
    {
      if ( syncFlag == 0) //no synchronization on
      {
         add((long long *) counter, 1); //add counter by 1
       
      }
       else if ( syncFlag && spinFlag)
      {
        
        spin_lock(&test1lock);
        add((long long *) counter, 1);
        spin_unlock(&test1lock);
      }
       else if ( syncFlag && mutexFlag) //mutex flag is on
      {
        

        pthread_mutex_lock(&lock);
        add((long long *) counter, 1);
        pthread_mutex_unlock(&lock);

      }
      else
      {
        //fprintf(stderr, "Before long long new old.\n");

        long long new, old;
        do{

          //fprintf(stderr, "Inside do-while. Before assignment.\n");

          old = *(long long *)counter;
          new = old + 1;
            if (opt_yield)
            {
              sched_yield();
            }
            
          //fprintf(stderr, "Inside do-while. After assignment.\n");
        }while(__sync_val_compare_and_swap((long long *)counter, old, new) !=  old);

        //fprintf(stderr, "Outside do-while loop.\n");
      }


      i++;
    }

  int j=0; 
  while( j < iterationNum)
  {
    if ( syncFlag == 0) //no synchronization on
      {
         add((long long *) counter,-1); //add counter by 1
       
      }
    else if ( syncFlag && spinFlag)
      {
        
        spin_lock(&test1lock);
        add((long long *) counter, -1);
        spin_unlock(&test1lock);
      }
    else if ( syncFlag && mutexFlag) //mutex flag is on
      {
        
        pthread_mutex_lock(&lock);
        add((long long *) counter, -1);
        pthread_mutex_unlock(&lock);
      }
    else
      {
        //fprintf(stderr, "Before long long new old.\n");

        long long new, old;
        do{

          //fprintf(stderr, "Inside do-while. Before assignment.\n");

          old = *(long long *)counter;
          new = old - 1;
            if (opt_yield)
            {
              sched_yield();
            }

          //fprintf(stderr, "Inside do-while. After assignment.\n");
        }while(__sync_val_compare_and_swap((long long *)counter, old, new) !=  old);

        //fprintf(stderr, "Outside do-while loop.\n");
      }


    j++;

  }

  pthread_exit(0);
}


// void add_compare(long long *pointer, long long value)
// {
//   long long prev;
//   long long sum;
//   do
//   {
//     prev = *pointer;
//     sum = prev + value;
//     if(opt_yield)
//       pthread_yield();
//   } while(__sync_val_compare_and_swap(pointer, prev, sum) != prev);
// }

// // Function that the threads will run
// void* threadAdd(void* counter)
// {
//   // Adds 1 to counter
//   int i;
//   for (i = 0; i < iterationNum; i++)
//   {
//     // Mutex locking
//     if (sync == 'm')
//     {
//       pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //LOCK INITIALIZED
//       pthread_mutex_lock(&lock);
//       add((long long*) counter, 1);
//       pthread_mutex_unlock(&lock);
//     }
//     // Spin locking
//     else if (sync == 's')
//     {
//       while(__sync_lock_test_and_set(&exclusion, 1));
//       add((long long*) counter, 1);
//       __sync_lock_release(&exclusion);
//     }
//     // Compare and swap
//     else if (sync == 'c')
//     {
//       add_compare((long long*) counter, 1);
//     }
//     // Regular add
//     else
//     {
//       add((long long*) counter, 1);
//     }
//   }

//   // Adds -1 to counter
//   int j;
//   for (j = 0; j < iterationNum; j++)
//   {
//     // Mutex locking
//     if (sync == 'm')
//     {
//       pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //LOCK INITIALIZED
//       pthread_mutex_lock(&lock);
//       add((long long*) counter, -1);
//       pthread_mutex_unlock(&lock);
//     }
//     // Spin locking
//     else if (sync == 's')
//     {
//       while(__sync_lock_test_and_set(&exclusion, 1));
//       add((long long*) counter, -1);
//       __sync_lock_release(&exclusion);
//     }
//     // Compare and swap
//     else if (sync == 'c')
//     {
//       add_compare((long long*) counter, -1);
//     }
//     // Regular add
//     else
//     {
//       add((long long*) counter, -1);
//     }
//   }
//   return NULL;
// }



int main( int argc, char *argv[]) //basic command line interface
{

  init(&test1lock);
  long long counter = 0;

  static struct option long_options[]={
    {"threads", required_argument, 0, threadOption}, //named threads
    {"iterations", required_argument, 0, iterationOption}, //named iterations
    {"yield", no_argument, 0, yieldOption},
    {"sync", required_argument, 0, syncOption},
    {"debug", no_argument, 0, 'd'}, //debug option for my own sake
    {0,0,0,0}


  };


  int ret = 0;
  while (1)
    {
      ret = getopt_long(argc, argv, "", long_options,0);
      if ( ret == -1)
	break;
      //while all command line arguments have not been parsed

      //then we'll do this

      switch (ret)
	{
	case threadOption:
	  threadNum = atoi(optarg);
	  break;
	case iterationOption:
	  iterationNum = atoi(optarg);
	  break;
  case yieldOption:
      opt_yield = 1; //if yield is provided, turn on flag
      break;
  case syncOption:
      syncFlag = 1;
        switch(*optarg)
        {
          case 'm':
            mutexFlag = 1;
            break;
          case 's':
            spinFlag = 1;
            break;
          case 'c':
            casFlag =1;
            break;
          default:
            fprintf(stderr, "Error occurred parsing sync!\n");
            exit(1);
          }
          break;
	default:
	  fprintf(stderr, "Error occurred during parsing!\n");
	  exit(1);
	}

 }

  //intializing threads and doing cool stuff

  //let's start time right now
  struct timespec startTime;
  clock_gettime(CLOCK_MONOTONIC, &startTime);

  pthread_t * tid = malloc(threadNum * sizeof(pthread_t));
  if ( tid == NULL) //error checking if i was able to make my threads
    {

      fprintf(stderr, "Error occurred during pthread allocation!\n");
      exit(1); //sys call failure
    }


  //with my tid now, will pthread_create threadNum threads
  int i;
  for ( i = 0; i < threadNum; i++)
    {
      pthread_create(&tid[i], NULL, threadAdd, &counter);
    }

  for ( i = 0; i < threadNum; i++)
    {
      pthread_join(tid[i], NULL);
    }

  //after all threads join, call end time
  struct timespec endTime;
  clock_gettime(CLOCK_MONOTONIC, &endTime);

  long long operationNum = threadNum * iterationNum * 2; //one for add, one for subtract

  //will now calculate elapsed time

  //cite clock website 
  long long diff = secs2nano * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_nsec - startTime.tv_nsec;
  long long avgtime = diff / operationNum; //average time per operation
  int total = counter; //the total at the end of the run

  //FORMAT!
  /* NAME OF TEST, #OF THREADS, #OF ITER, #OF OPS, RUN TIME, AVG TIME PER OP, TOTAL       */

  //let's print the usual
  printf("add");

  if (opt_yield)
  {
    printf("-yield");
  }
  if ( mutexFlag)
  {
    printf("-m");
  }
  else if ( spinFlag)
  {
    printf("-s");
  }
  else if ( casFlag)
  {
    printf("-c");
  }
  else
  {  
  printf("-none");
  }

  printf(",%d,%d,%lld,%lld,%lld,%lld\n", threadNum, iterationNum, operationNum, diff,avgtime,total);

  free(tid);

  
    exit(0);
  
  
}
