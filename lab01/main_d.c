/*  main.c  - main */

#include <xinu.h>

pid32 producer_id;
pid32 consumer_id;
pid32 timer_id;

int32 consumed_count = 0;
const int32 CONSUMED_MAX = 100;

/* Define your circular buffer structure and semaphore variables here */

int32 n = 0;
sid32 mutex,count,empty; /* mutex for mutual exclusion- count: number of valid values in Circular queue - empty: number of empty slots in CQ */

#define SIZE 5            
int CQ[SIZE],f=-1,r=-1; 

 
void CQinsert(int elem)
{                       /* Function for Insert operation */
    
        if(f==-1)f=0;
        r=(r+1) % SIZE;
        CQ[r]=elem;
}
int32 CQdelete()
{                      /* Function for Delete operation */
    int32 elem;
        elem=CQ[f];
        if(f==r){ f=-1; r=-1;} /* Q has only one element ? */
        else
            f=(f+1) % SIZE;
        return(elem);
}


/* */

/* Place your code for entering a critical section here */
void mutex_acquire(sid32 mutex)
{
	/* */
	wait(mutex);
}

/* Place your code for leaving a critical section here */
void mutex_release(sid32 mutex)
{
	/* */
	signal(mutex);
}

/* Place the code for the buffer producer here */
process producer(void)
{

	int32 i;
	int32 p;
	for( i=0 ; i<= 1600 ; i++ ) {
		mutex_acquire(empty);
		mutex_acquire(mutex);
		p = ++n;
		CQinsert(p);
		printf("produced %d \n", p);
		mutex_release(mutex);
		mutex_release(count);
	}
	return OK;
}

/* Place the code for the buffer consumer here */
process consumer(void)
{
	/* Every time your consumer consumes another buffer element,
	 * make sure to include the statement:
	 *   consumed_count += 1;
	 * this will allow the timing function to record performance */

	 	int32 i;
	 for( i=0 ; i<= 1600 ; i++ ) {
	 	mutex_acquire(count);
		 mutex_acquire(mutex);
		 printf("consumed %d \n", CQdelete());
		 consumed_count += 1;
		 mutex_release(mutex);
		 mutex_release(empty);
	 }
	return OK;
}


/* Timing utility function - please ignore */
void time_and_end(void)
{
    int32 times[5];
    int32 i;

    for (i = 0; i < 5; ++i)
    {
        times[i] = clktime_ms;
        yield();
        consumed_count = 0;
        while (consumed_count < CONSUMED_MAX * (i+1))
        {
            yield();
        }

        times[i] =  clktime_ms - times[i];
    }

    kill(producer_id);
    kill(consumer_id);

    for (i = 0; i < 5; ++i)
    {
        kprintf("TIME ELAPSED (%d): %d\n", (i+1) * CONSUMED_MAX, times[i]);
    }
}

process	main(void)
{
	kprintf("The main has started\n");
	recvclr();

	/* Create the shared circular buffer and semaphores here */
	/* */
	mutex  = semcreate(1);
	count = semcreate(0);
	empty = semcreate(SIZE);

	producer_id = create(producer, 4096, 50, "producer", 0);
	consumer_id = create(consumer, 4096, 50, "consumer", 0);
	timer_id = create(time_and_end, 4096, 50, "timer", 0);

	resched_cntl(DEFER_START);
	resume(producer_id);
	resume(consumer_id);
	/* Uncomment the following line for part 3 to see timing results */
	resume(timer_id);
	resched_cntl(DEFER_STOP);

	return OK;
}