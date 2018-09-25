#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include "cthread.h"
#include "support.h"
#include "cdata.h"



int ccreate (void *(*start) (void *), void *arg, int prio)
{	TCB_t* new_thread;


	new_thread = (TCB_t*) malloc(sizeof(TCB_t));

	/* Making thread context */
	getcontext(&new_thread->context);
	new_thread->context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
	new_thread->context.uc_stack.ss_size = SIGSTKSZ;
	new_thread->context.uc_link = &control.ended_thread;
	makecontext(&new_thread->context, (void (*)(void))start, 1, arg);


	/* Put it into all_treads and able_threads */
	if (!rb_insert(control.all_threads, new_thread->tid, new_thread))
		return FALSE;
	if (!rb_able_insert(new_thread->tid))
		return FALSE;


return new_thread->tid;
}