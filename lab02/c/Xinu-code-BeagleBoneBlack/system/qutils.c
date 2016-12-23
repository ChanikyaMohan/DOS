#include <xinu.h>

int32 CQfull(pid32);
int32 CQempty(pid32);
int32 CQcount(pid32);
void CQinsert(pid32,int32);
int32 CQdelete(pid32);

void initialize_proctab(int32);


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

/* function to initialize the front rear and msgcount of the process entry */
void initialize_proctab(int32 id){ // id - process id of the process entry to be initialized
  struct procent * entry; 
  entry = &proctab[id];
  entry->f = -1;
  entry->r = -1;
  entry->prmsgcount = 0;
}
