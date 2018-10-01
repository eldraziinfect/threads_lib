#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include "cthread.h"
#include "support.h"
#include "cdata.h"

/* Funções auxiliares */

/* Variáveis Globais */

/* Filas */
FILA2 aptoAltaPrior
PFILA2 APTO_ALTA = &aptoAltaPrior
FILA2 aptoMediaPrior
PFILA2 APTO_MEDIA = &aptoMediaPrior
FILA2 aptoBaixaPrior
PFILA2 APTO_BAIXA = &aptoBaixaPrior
FILA2 filaBloqueado
PFILA2 BLOQUEADO = &filaBloqueado
FILA2 filaExecutando
PFILA2 EXECUTANDO = &filaExecutando
FILA2 filaJoin
PFILA2 JOIN = &filaJoin

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

int cyield(void){
	return 0;
}

int csetprio(int tid, int prio);

int cjoin(int tid);

int csem_init(csem_t *sem, int count);

int cwait(csem_t *sem);

int csignal(csem_t *sem);

int cidentify (char *name, int size);
