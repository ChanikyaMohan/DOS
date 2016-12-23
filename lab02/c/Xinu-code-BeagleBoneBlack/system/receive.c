/* receive.c - receive */

#include <xinu.h>

sid32 recMutex;
/*------------------------------------------------------------------------
 *  receive  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receive(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &proctab[currpid];
	if (prptr->prhasmsg == FALSE) {
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}
	msg = prptr->prmsg;		/* Retrieve message		*/
	prptr->prhasmsg = FALSE;	/* Reset message flag		*/
	restore(mask);
	return msg;
}


/* Reading a msg always from prptr->prmsg and dequeuing the next msg from queue and add it to prmsg */
umsg32 receiveMsg(void){
	intmask mask; /* Saved interrupt mask */
	struct procent *prptr; /* Ptr to process’ table entry */
	umsg32 msg; /* Message to return */
	mask = disable();
	prptr = &proctab[currpid];
	if(CQempty(currpid)){	
			prptr->prstate = PR_RECV;
			resched(); /* Block until message arrives */
	}
	//msg = prptr->prmsg; /* Retrieve message */
	          /* add queue msg into prmsg if any are present in queue */
		//prptr->prmsg = CQdelete(currpid);
	
		msg = CQdelete(currpid);
		prptr->prmsgcount--;
		//prptr->prhasmsg = TRUE;
		kprintf("------%d received %d\n",msg,currpid);
		restore(mask);
		return msg;
	
	//prptr->prhasmsg = FALSE; /* Reset message flag */
	//restore(mask);
	//kprintf("------%d received %d\n",msg,currpid);
	//return msg;
}

syscall receiveMsgs(umsg32* msgs,uint32 msg_count){
	int32 i;
	intmask mask; /* Saved interrupt mask */
	struct procent *prptr; /* Ptr to process’ table entry */
	mask = disable();
	prptr = &proctab[currpid];
	if ( prptr->prmsgcount < msg_count) { /* wait untill msg_count number of messages arrive */
		prptr->prstate = PR_RECV;
		resched(); /* Block until message arrives */
	}
	for(i=0;i< msg_count;i++){
		 msgs[i] = CQdelete(currpid);
		 prptr->prmsgcount--;//decrement message count in array
	} /* Retrieve message */
	if(!CQempty(currpid)){          /* if queue is not empty prhasmsg flag should be true */
		//prptr->prhasmsg = TRUE;
		restore(mask);
		return OK;
	} 
	//prptr->prhasmsg = FALSE; /* Reset message flag - if queue is empty*/
	restore(mask);
	return OK;
}


