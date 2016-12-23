/* send.c - send */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  send  -  Pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	send(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* Contents of message		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prhasmsg) {
		restore(mask);
		return SYSERR;
	}
	prptr->prmsg = msg;		/* Deliver message		*/
	prptr->prhasmsg = TRUE;		/* Indicate message is waiting	*/

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return OK;
}



syscall sendMsg(pid32 pid, umsg32 msg){
	intmask mask; /* Saved interrupt mask */
	struct procent *prptr; /* Ptr to process’ table entry */
	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}
	prptr = &proctab[pid];
	if ((prptr->prstate == PR_FREE) ) {
		restore(mask);
		return SYSERR;
	}
	if(prptr->prhasmsg){ // if process has msg in prmsg - add it to queue
		if( CQfull(pid)){
			kprintf("\n Overflow!!!! \n");
			resched(); // allow receiver to receive msgs
		}
		CQinsert(pid,msg);  //insert msg into queue and prhasmsg is already true	
		kprintf("%d msg sent to %d process\n",msg,pid); 
		restore(mask);
		return OK;
	}
	//using prmsg as a intermediate spot for msg
	prptr->prmsg = msg; /* Deliver message */
	kprintf("%d msg sent to %d process\n",msg,pid);
	prptr->prhasmsg = TRUE; /* Indicate message is waiting */
	/* If recipient waiting or in timed-wait make it ready */
	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask); /* Restore interrupts */
	return OK;
}



uint32 sendMsgs(pid32 pid, umsg32* msgs, uint32 msg_count){
	int32 i;
	intmask mask; /* Saved interrupt mask */
	struct procent *prptr; /* Ptr to process’ table entry */
	mask = disable();
	if (isbadpid(pid)) {
	restore(mask);
	return SYSERR;
	}
	prptr = &proctab[pid];
	if ((prptr->prstate == PR_FREE) ) {
		restore(mask);
		return SYSERR;
	}
	
	/*if(msg_count> (SIZE - prptr->prmsgcount) ){ /* checking whether the free space can fit the incoming messages 
		resched();
	}*/
		for(i=0;i< msg_count;i++){
			if( CQfull(pid)){
				kprintf("\n Overflow!!!! msg not sent to process %d\n",pid);
				return SYSERR;
			}
			CQinsert(pid,msgs[i]);
			kprintf("%d msg sent to %d",msgs[i],pid);
			prptr->prmsgcount++;//increment message count in array
		} /* Deliver message */	
	
	prptr->prhasmsg = TRUE; /* Indicate message is waiting*/
	/* If recipient waiting or in timed-wait make it ready */
	if (prptr->prstate == PR_RECV) {
	ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
	unsleep(pid);
	ready(pid);
	}
	restore(mask); /* Restore interrupts */
	return msg_count;
}


uint32 sendnMsg(uint32 pid_count, pid32* pids,umsg32 msg){
	intmask mask; /* Saved interrupt mask */
	pid32 pid;
	struct procent *prptr; /* Ptr to process’ table entry */
	int32 i;
	uint32 sent = 0;
	mask = disable();
	
	for(i=0; i< pid_count;i++){
		pid = pids[i];

		prptr = &proctab[pid];
		if ((prptr->prstate == PR_FREE) || isbadpid(pid)) {
			kprintf("Msg not sent to %d", pid );
			break;
		}
		if(prptr->prhasmsg){ //insert into queue
			if( CQfull(pid)){
				kprintf("\n Overflow!!!! \n");
				resched(); // allow receiver to receive msgs
			}
			CQinsert(pid,msg);  //insert msg into queue and prhasmsg is already true
			sent++;
			kprintf("%d msg sent to process %d\n", msg, pid);
			break;
		}
		prptr->prmsg = msg; /* Deliver message */
		sent++;
		kprintf("%d msg sent to process %d\n", msg, pid);
		prptr->prhasmsg = TRUE; /* Indicate message is waiting */
		/* If recipient waiting or in timed-wait make it ready */
		if (prptr->prstate == PR_RECV) {
			ready(pid);
		} else if (prptr->prstate == PR_RECTIM) {
			unsleep(pid);
			ready(pid);
		} 
	}
	restore(mask); /* Restore interrupts */
	return sent;
}

