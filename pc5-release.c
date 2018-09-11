#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>    
#include <string.h>

#ifndef _REENTRANT
#define _REENTRANT
#endif

#define BUFF_SIZE   20		 
#define NP          3		/* total number of producers */
#define NC          4		/* total number of consumers */
#define NITEMS      200		/* number of items produced/consumed */

sem_t full;			/* semaphore,keeping track of the number of full slots */
sem_t empty;			/* semaphore,keeping track of the number of empty slots */
//sem_t pmutex; 
//sem_t cmutex; 
pthread_mutex_t mutex;	/* enforce mutual exclusion to shared data */


struct
{
  
  int in;			/* buf[in] is the first empty slot */
  int total_in;
  int out;			/* buf[out] is the first full slot */
  int total_out;
  int buf[BUFF_SIZE];	 
  
} shared;

  struct recType
  {
    pthread_t tid;
    int no;
    int numitems;
  } Producers[NP], Consumers[NC];


void *
producer_runner (void *arg)
{
  int i, item;
  struct recType *p;
    
  p = (struct recType *) arg;

  while (1)
    {
    
      sem_wait (&empty);/* If there are no empty slots, wait */            
      pthread_mutex_lock(&mutex);
      
      if (shared.total_in >= NITEMS)
	{
	  sem_post (&empty);	/* Increment the number of full slots */	  
	  pthread_mutex_unlock(&mutex);	  
	  pthread_exit(0);
	}
	else{
      item = shared.total_in+1;
      shared.buf[shared.in] = item;      
      shared.total_in += 1;////total Produced 
	  fprintf (stdout,"[P_%d] Producing no.%3d item into buf[%2d].\n", p->no, item, shared.in);
	  fflush (stdout);
	  shared.in = (shared.in + 1) % BUFF_SIZE;      
      p->numitems += 1;// Produced by this thread     
            
      sem_post (&full);
      pthread_mutex_unlock(&mutex);	 
	}

    }
}

void *
consumer_runner (void *arg)
{
  int i, item;
  struct recType *p;
  
  p = (struct recType *) arg;  

  while (1)
    {
      sem_wait (&full); // if item ready in buffer
      pthread_mutex_lock(&mutex);

      if (shared.total_out >= NITEMS )
	  {
	    sem_post (&full);	  
	    pthread_mutex_unlock(&mutex);	    
	    pthread_exit(0);
	  } 	
      item = shared.buf[shared.out];      
      shared.buf[shared.out]=-1;
      shared.total_out += 1; //total comsumed 
      fprintf (stdout,"[C_%d] Consuming no.%3d item from buf[%2d]\n", p->no, item,shared.out);
      fflush (stdout);
      shared.out = (shared.out + 1) % BUFF_SIZE;      
      p->numitems += 1;  // consumed by this thread 

      if (shared.total_out >= NITEMS )
	  {
	    sem_post (&full);	  
	    pthread_mutex_unlock(&mutex);	    
	    pthread_exit(0);
	  } 
	  else{ 
      	sem_post (&empty);
      	pthread_mutex_unlock(&mutex); 
	  }

    }
}

int
main ()
{

  int i; 
  int res;
  
  sem_init (&full, 0, 0);
  sem_init (&empty, 0, BUFF_SIZE);

  res=pthread_mutex_init(&mutex, NULL);
  if(res!=0)
  {
  	perror("Mutex initialize error.\n");
  	exit(EXIT_FAILURE);
  }
  
  shared.in=shared.out=0;
  shared.total_in=shared.total_out=0;
  memset(shared.buf,0,sizeof(int)*BUFF_SIZE);
      
  /* Create new producers */
  for (i = 0; i < NP; i++)
    {
      Producers[i].numitems = 0;
      Producers[i].no=i;
      pthread_create (&(Producers[i].tid), NULL, producer_runner, (void *) (&(Producers[i])));
    }
  /*create new Consumers */
  for (i = 0; i < NC; i++)
    {
      Consumers[i].numitems = 0;
      Consumers[i].no=i;
      pthread_create (&(Consumers[i].tid), NULL, consumer_runner, (void *) (&(Consumers[i])));
    }

  
  printf("\n");
  for (i = 0; i < NP; i++)
    {      
      fflush (stdout);
      pthread_join (Producers[i].tid, NULL);
      fprintf (stdout,"The Producer (%d)  produced:[%d] Items \n", Producers[i].no, Producers[i].numitems);	  
      sleep(1);
    }
  fprintf (stdout,"shared.total_in: %d\n\n", shared.total_in) ;      

	  sleep (1);
  
  for (i = 0; i < NC; i++)
    {      
      pthread_join (Consumers[i].tid, NULL);
      fprintf (stdout,"The Consumer (%d)  consumed:[%d] Items \n",  Consumers[i].no, Consumers[i].numitems);
	  fflush (stdout);
	  sleep(1);
    }
    
    fprintf (stdout,"shared.total_out: %d\n", shared.total_out) ;
  pthread_exit (NULL);  
}
