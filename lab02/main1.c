/*  main.c  - main */
#include <xinu.h>

pid32 senderID,receiverID;
sid32 sendsem,receivesem;

int32 CQfull(pid32);
int32 CQempty(pid32);
int32 CQcount(pid32);
void CQinsert(pid32,int32);
int32 CQdelete(pid32);

void receiver(sid32 , sid32 );
void sender(sid32 , sid32 );


int32  CQfull(pid32 pid)
{   struct procent *prptr; /* Ptr to process’ table entry */
	prptr = &proctab[pid];                 /* Function to Check Circular Queue Full */
    if( (prptr->f==prptr->r+1) || (prptr->f == 0 && prptr->r== SIZE-1)) return 1;
    return 0;
}
 
int32 CQempty(pid32 pid)
{   struct procent *prptr; /* Ptr to process’ table entry */
	prptr = &proctab[pid];                 /* Function to Check Circular Queue Empty */
    if(prptr->f== -1) return 1;
    return 0;
}

void CQinsert(pid32 pid,int32 elem)
{      struct procent *prptr; /* Ptr to process’ table entry */
		prptr = &proctab[pid];                 /* Function for Insert operation */
          if(prptr->f==-1)prptr->f=0;
          prptr->r=(prptr->r+1) % SIZE;
          prptr->prmsgs[prptr->r]=elem;
}
int32 CQdelete(pid32 pid)
{   struct procent *prptr; /* Ptr to process’ table entry */
		prptr = &proctab[pid];                   /* Function for Delete operation */
   		int32 elem;
          elem=prptr->prmsgs[prptr->f];
          if(prptr->f==prptr->r){ prptr->f=-1; prptr->r=-1;} /* Q has only one element ? */
          else
              prptr->f=(prptr->f+1) % SIZE;
          return(elem);
}


void sender(sid32 sendsem, sid32 receivesem)
{
	int32 i = 0;
	while( i<20 ){
		wait(sendsem);
		sendMsg(receiverID, i);
		kprintf("%d msg sent to process %d\n", i, receiverID);	
		i++;
		signal(receivesem);
	}
}
void receiver(sid32 sendsem, sid32 receivesem)
{
	int32 i = 0;	
	while( i<20 )
	{	
		wait(receivesem);
		kprintf("received msg %d\n",receiveMsg());
		i++;
		signal(sendsem);
	}
}

/* function to initialize the front rear and msgcount of the process entry */
void initialize_proctab(int32 id){ // id - process id of the process entry to be initialized
	struct procent * entry; 
	entry = &proctab[id];
	entry->f = -1;
	entry->r = -1;
	entry->prmsgcount = 0;
}

process	main(void)
{	
	kprintf("The main has executed\n");
	recvclr();
	
	sendsem = semcreate(1);
	receivesem = semcreate(0);

	senderID = create(sender, 1024, 20, "process 1", 2, sendsem,receivesem); 
	initialize_proctab(senderID) ;
	
	receiverID = create(receiver, 1024, 20, "process 2", 2, sendsem,receivesem);
	initialize_proctab(receiverID);
		
	resume( senderID );
	resume( receiverID );
	
	
	return OK;
}


