/*  main.c  - main */
#include <xinu.h>

sid32 console_lock;
pid32 comm_a_pid, comm_b_pid;

#define gassert(x) grade_assert((x), #x, NULL, 1)
#define gmassert(x, m) grade_assert((x), #x, m, 1)
#define gmassert_opt(x, m) grade_assert((x), #x, m, 0)

int grading = 0;
void grade_assert(int cond, const char* code, const char* msg, int inc)
{
	wait(console_lock);

	grading += inc;

	if (cond)
		kprintf("[ * ] ");
	else
		kprintf("[ X ] ");

	kprintf("%d. ", grading);

	if (msg)
		kprintf("%s ", msg);

	kprintf("(%s)\n", code);

	signal(console_lock);
}

process dummy(void) { for (;;); return OK; }

int single_done = 0;
process single(void)
{
	// Test single receives
	umsg32 a = receiveMsg();
	umsg32 b = receiveMsg();
	umsg32 c = receiveMsg();

	gassert(a == 3);
	gassert(b == 4);
	gassert(c == 1);

	// Test empty single receive
	receiveMsg();
	gmassert(0, "Should never be reached");

	single_done = 1;
	return OK;
}

int multi_done = 0;
process multi(void)
{
	// Test multi receives
	umsg32 abc[3];
	receiveMsgs(abc, 3);

	gassert(abc[0] == 6);
	gassert(abc[1] == 7);
	gassert(abc[2] == 1);

	// Test empty multi receive
	receiveMsgs(abc, 2);
	gmassert(0, "Should never be reached");

	multi_done = 1;
	return OK;
}

int comm_a_done = 0;
process comm_a(void)
{
	// Test sending and receiving response
	sendMsg(comm_b_pid, 2);
	sendMsg(comm_b_pid, 5);

	gassert(receiveMsg() == 7);

	// Test sending to self
	sendMsg(comm_a_pid, 8);
	gassert(receiveMsg() == 8);

	comm_a_done = 1;
	return OK;
}

process comm_b(void)
{
	for (;;)
	{
		umsg32 a = receiveMsg();
		umsg32 b = receiveMsg();

		sendMsg(comm_a_pid, a + b);
	}

	return OK;
}

process main(void)
{
	recvclr();
	console_lock = semcreate(1);

	resched_cntl(DEFER_START);
	{
		// Test simple sending success
		pid32 dummy1 = create(dummy, 4096, 50, "dummy1", 0);
		gassert(sendMsg(dummy1, 7) == OK);

		int i;
		for (i = 0; i != 9; ++i)
			sendMsg(dummy1, 8);

		// Test simple sending error
		gassert(sendMsg(dummy1, 7) == SYSERR);

		// Test invalid simple sending
		gassert(sendMsg(0xDEADBEEF, 3) == SYSERR);

		umsg32 msg3[3] = { 3, 4, 5 };
		umsg32 msg8[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
		umsg32 msg12[12];

		// Test multi sending success
		pid32 dummy2 = create(dummy, 4096, 50, "dummy2", 0);
		gassert(sendMsgs(dummy2, msg3, 3) == 3);

		// Test multi sending error
		uint32 d2r = sendMsgs(dummy2, msg8, 8);
		gmassert(d2r == 7, "Preferred");
		gmassert_opt(d2r == SYSERR, "Acceptable");

		// Test invalid multi sending
		gassert(sendMsgs(0xDEADBEEF, msg3, 3) == SYSERR);

		// Test multi sending over limit
		gmassert(sendMsgs(dummy2, msg12, 12) == SYSERR, "Noncritical");

		pid32 dummy3 = create(dummy, 4096, 50, "dummy3", 0);
		pid32 dummy4 = create(dummy, 4096, 50, "dummy4", 0);
		pid32 dummy5 = create(dummy, 4096, 50, "dummy5", 0);

		pid32 pid3a[3] = { dummy3, dummy4, dummy5 };
		pid32 pid3b[3] = { dummy3, dummy4, dummy2 };
		pid32 pid3c[3] = { dummy3, dummy4, 0xDEADBEEF };
		pid32 pid4[4]  = { dummy3, dummy4, dummy5, dummy2 };

		// Test combo sending success
		gassert(sendnMsg(2, pid3a, 9) == 2);

		// Test combo sending error
		uint32 d342r = sendnMsg(3, pid3b, 9);
		gmassert(d342r == 2, "Preferred");
		gmassert_opt(d342r == SYSERR, "Acceptable");

		// Test invalid combo sending
		uint32 d34dr = sendnMsg(3, pid3c, 9);
		gmassert(d34dr == 2, "Preferred");
		gmassert_opt(d34dr == SYSERR, "Acceptable");

		// Test combo sending over limit
		gmassert(sendnMsg(4, pid4, 8) == SYSERR, "Noncritical");

		pid32 single_pid = create(single, 4096, 50, "single", 0);
		pid32 multi_pid  = create(multi,  4096, 50, "multi",  0);

		pid32 pid2[2] = { single_pid, multi_pid };

		// Set up single recv test
		sendMsgs(single_pid, msg3, 2);

		// Set up multi recv test
		sendMsg(multi_pid, 6);
		sendMsg(multi_pid, 7);

		// Top off both
		sendnMsg(2, pid2, 1);

		// Let two processes communicate
		comm_a_pid = create(comm_a, 4096, 50, "comm_a", 0);
		comm_b_pid = create(comm_b, 4096, 50, "comm_b", 0);

		resume(single_pid);
		resume(multi_pid);

		resume(comm_a_pid);
		resume(comm_b_pid);
	}
	resched_cntl(DEFER_STOP);

	return OK;
}
