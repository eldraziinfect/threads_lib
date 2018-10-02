#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "cthread.h"
#include "support.h"
#include "cdata.h"

/* Funções auxiliares */
TCB_t* searchTID(PFILA2 pFila, int tid);
TCB_t* pickHighestPriority();

/* Variáveis Globais */
int tid = 0;



/* Filas */
FILA2 aptoAltaPrior;
PFILA2 APTO_ALTA = &aptoAltaPrior;
FILA2 aptoMediaPrior;
PFILA2 APTO_MEDIA = &aptoMediaPrior;
FILA2 aptoBaixaPrior;
PFILA2 APTO_BAIXA = &aptoBaixaPrior;
FILA2 filaBloqueado;
PFILA2 BLOQUEADO = &filaBloqueado;
FILA2 filaExecutando;
PFILA2 F_EXECUTANDO = &filaExecutando;
FILA2 filaJoin;
PFILA2 JOIN = &filaJoin;


/* Ponteiro para o TCB em execução */
TCB_t *EXECUTANDO;

/* Variável u_context para facilitar o retorno */
ucontext_t r_context;


int ccreate (void *(*start) (void *), void *arg, int prio)
{
	if(tid ==0){ bksbakjdba}
	tid+=1;
	TCB_t* new_thread2 = (TCB_t*) malloc (sizeof(TCB_t));
	
	new_thread2->prio = prio;
	new_thread2->state = PROCST_APTO;
	new_thread2->tid = tid;
	if (getcontext(&new_thread2->context) == -1)
	    return -1;

	//Inicializa contexto
	(new_thread2->context).uc_link = &returncontext;
	(new_thread2->context).uc_stack.ss_sp = malloc (TAM_MEM * sizeof(char));
	(new_thread2->context).uc_stack.ss_size = TAM_MEM;
	makecontext(&(new_thread2->context), (void(*)())start,1,arg);

	//ao final do processo de criação, a thread deverá ser inserida na fila dos aptos
	if(EXCUTANDO->prio < new_thread2->prio){
		despachante(new_thread2);
	}
	
	if (AppendFila2(APTO, novo_tcb))
	    return -1;

	return tid;
}

//////////////////////////////******************************
// inserirApto(EXECUTANDO) em qual das filas dos aptos vai inserir???? ja q temos 3
// Cedência voluntária:
int cyield(void){
	EXECUTANDO->state = PROCST_APTO; //Passa de executando pra apto.
	if(inserirApto(EXECUTANDO)) // retorna 0 caso tenha obtido sucesso, igual ao AppendFila2
		return -1;
	else{
		escalonador();
		return 0;
	}
}

// Setar prioridade:
int csetprio(int tid, int prio){
	//Aparentemente é pra deixar tid em null em 2018/2, então o csetprio funciona apenas na thread corrente.
	if(prio >= 0 && prio <= 2){
		if(EXECUTANDO->prio > prio){
		// Executar caso o processo tenha baixado a prioridade, executar o escalonador
		return 0;
		}
		else{
			EXECUTANDO->prio = prio;
			return 0;
		}
	}
	else //valor de prioridade é inválido, deve estar entre [0,2].
		return -1;
}

int cjoin(int tid){
/*
* Verificar a existência da thread
* Verificar se a thread já está sendo esperada
*/
if((procurarApto) && (searchTID(BLOQUEADO,tid) == NULL))
	return -1;

}

// Semáforo
int csem_init(csem_t *sem, int count);
// Semáforo
int cwait(csem_t *sem);
// Semáforo
int csignal(csem_t *sem){
	//para cada chamadada primitiva a variável count deve ser decrementada 
	// de uma unidade
	sem->count += 1;


    return 0;
}

int cidentify (char *name, int size){;
		char nomes[] = "Carine Bertagnolli Bathaglini - 00274715\nGabriel Pakulski da Silva - 00274701\nLuiz Miguel Kruger - 00228271\n";
		int i,flag = 0;
		for(i = 0; i < size; i++){
            if(nomes[i] == '\0'){
                flag = 1;
                break;
            }
		}
		if(flag == 0) return -1;
		for(i = 0;i < size && nomes[i] != '\0';i++){
                name[i] = nomes[i];
		}
		name[i] = '\0';
		return 0;
}


/********************* FUNÇÕES AUXILIARES *********************/


void init_threads(){
	//caso em que as filas falharam em sua criação
	if(CreateFila2(APTO_ALTA)) { 
		printf("ERRO: falha ao criar fila dos aptos de prioridade alta\n");
		return -1;
	}
	if(CreateFila2(APTO_MEDIA)){
		printf("ERRO: falha ao criar fila dos aptos de prioridade média\n");
		return -1;
	}
	if(CreateFila2(APTO_BAIXA)){
		printf("ERRO: falha ao criar fila dos aptos de prioridade baixa\n");
		return -1;
	}
	if(CreateFila2(BLOQUEADO)){
		printf("ERRO: falha ao criar fila dos bloqueados\n");
		return -1;
	}
	if(CreateFila2(APTO_F_EXECUTANDO)){
		printf("ERRO: falha ao criar fila join\n");
		return -1;
	}

	TCB_t* new_thread = (TCB_t*) malloc(sizeof(TCB_t));

	new_thread->prio = prio; // prioridade da thread é passada no parâmetro
	new_thread->tid = tid;
	new_thread->state = PROCST_APTO; // a thread está apta

	EXECUTANDO = new_thread;

	//create a new thread: getcontext to create a valid context 
	//(leave the current thread running)
	getcontext(&(new_thread->context));
	if(getcontext(&r_context) == -1) return -1;
	if (lockReturn == 1){
	    clearBlocked(EXECUTANDO->tid);
		//readFila(BLOCK);
	    EXECUTANDO->state = PROCST_TERMINO;

	    //VERIFICAR SE CHEGOU AO FIM DA MAIN PARA DESALOCAR FILAS/SEMAFOROS

	    escalonador();
	    return 0;
	}
	lockReturn = 1;
}
	
}


/* A thread sai do modo bloqueado e vai para o estado apto 
   RETURN: 1 - caso tenha retirado da fila dos bloqueados
          -1 - caso não tenha retirado alguém da fila dos aptos
*/
int freeBlocked(int tidJ){	
	/*Retorna 0 se retirou alguém da fila de blocked for join, -1 se não.*/
    wJoin_t* join;
    TCB_t* free_thread;

    if (FirstFila2(JOIN))  return -1; 

	/*Procura pelo tidJ (tid do TCB que está sendo esperado)
	para verificar se tem alguém bloqueado por ele*/
    while((join =GetAtIteratorFila2(W_JOIN)) != NULL){
        if(join->tidJoin == tidJ){	//Se tiver na fila, coloca o que estava bloqueado para apto
            free_thread = searchTID(BLOCK,join->tidBlocked);
            free_thread->state = PROCST_APTO; // muda o estado dessa thread para apto
		
	    free_thread
	
            if(AppendFila2(APTO, tcb_aux)) return -1; // falhou ao ser colocada na fila dos aptos
            if(deleteNode(BLOCK, tcb_aux)) return -1; // falhou ao tentar ser deletada dos bloqueados
            if(DeleteAtIteratorFila2(W_JOIN)) return -1;

            return 0;
        }
        else
            if (NextFila2(W_JOIN))
                return -1;
    }
	return -1;
}

TCB_t* searchTID(PFILA2 fila, int tid)
 /*Procura numa fila se existe o processo de tid e retorna | um ponteiro para o TCB caso positivo
  																												 | NULL caso contrário*/
{
    TCB_t* tcb;
    if (FirstFila2(fila)) // fila vazia
	{	return NULL;
    }
	while(*tcb = GetAtIteratorFila2(fila)))
	{	if(tcb->tid == tid)
		{	return tcb;
        }
		else
		{	if (NextFila2(fila))
			{	return NULL;
			}
		}
    }
	return NULL;
}


int inserirApto(TCB_t* thread){
	switch(thread->prio){
		case 0: //inserir no APTO_ALTA
			if(AppendFila2(APTO_ALTA,thread)) // RETORNA 0 CASO DEU SUCESSO
				return -1;
			else
				return 0;
			break;
		case 1: //inserir no APTO_MEDIA
			if(AppendFila2(APTO_MEDIA,thread))
				return -1;
			else
				return 0;
			break;
		case 2: //inserir no APTO_BAIXA
			if(AppendFila2(APTO_BAIXA,thread))
				return -1;
			else
				return 0;
			break;
		default:
			printf("A PRIORIDADE DO PROCESSO TA ZUADA!\n");
			return -1;
			break;
	}
}

int escalonador(){
	/*
	* Implementação de um escalonador preemptivo por prioridade
	*/
	TCB_t *proximo = pickHighestPriority();
	if(proximo == NULL){
		printf("A fila de aptos se encontra vazia!\n");
		return -1; // -1 indica que a fila de aptos está vazia
	}
	if(proximo->prio > EXECUTANDO->prio){
		despachante(proximo);
		return 0; //retorna 0 caso a troca de contexto tenha ocorrido.
	}
	return 1; //retorna 1 caso não tenha ocorrido troca de contexto.
}

TCB_t* pickHighestPriority(){
	TCB_t *escolhido;
	if(FirstFila2(APTO_ALTA)){ // Verdadeiro se não tem nenhum elemento de prioridade alta
		if(FirstFila2(APTO_MEDIA)){ // Verdadeiro se não tem nenhum elemento de prioridade alta ou média:
			if(FirstFila2(APTO_BAIXA)){ //Verdadeiro se não tem nenhum elemento em nenhuma fila (alta, media e baixa)
				return -1;
			}
			else{ //Existe pelo menos um TCB de prioridade baixa.
				escolhido = GetAtIteratorFila2(APTO_BAIXA);
			}
		}
		else{//Existe pelo menos um TCB de prioridade media.
			escolhido = GetAtIteratorFila2(APTO_MEDIA);
		}
	}
	else{//Existe pelo menos um TCB de prioridade alta.
		escolhido = GetAtIteratorFila2(APTO_ALTA);
	}
	return escolhido;
}

//*******PEDIU UM RETORNO: COLOQUEI VOID
void despachante(TCB_t *proximo){
	int estado = EXECUTANDO->state; //o próximo estado do executando define qual tratamento ele receberá.
	switch(estado){
		case PROCST_APTO: //É necessário colocar o processo na fila de aptos.
								proximo->state = PROCST_EXEC;
								TCB_t *temp = EXECUTANDO;
								EXECUTANDO = proximo;
								if(removerApto(proximo))
									return -1;
								swapcontext(&(temp->context),&(EXECUTANDO->context));
								return 0;
								break;
		case PROCST_BLOQ: //é necessário colocar o processo na fila de bloqueados.
								if(AppendFila2(BLOQUEADO,EXECUTANDO))
									return -1;
								TCB_t *temp = EXECUTANDO;
								EXECUTANDO = proximo;
								if(removerApto(proximo))
									return -1;
								swapcontext(&(temp->context),&(EXECUTANDO->context));
								return 0;
								break;
		case PROCST_TERMINO: //o processo foi terminado: desalocar o PCB.
								free(EXECUTANDO);
								//EXEC = NULL;
								proximo->state = PROCST_EXEC;
								EXECUTANDO = proximo;
								if(removerApto(EXECUTANDO))
									return -1;
								setcontext(&(EXECUTANDO->context));
								break;
		default:
								return -1;
	}
}

//*******PEDIU UM RETORNO: COLOQUEI VOID
int adicionarApto (TCB_t *tcb){
	/* Retorna 0 caso tenha funcionado e -1 cc.
	*/
		switch(tcb->prio){
			case 0:
					if(AppendFila2(APTO_ALTA,tcb))
						return -1;
					else
						return 0;
					break;

			case 1:
					if(AppendFila2(APTO_MEDIA,tcb))
						return -1;
					else
						return 0;
					break;

			case 2:
					if(AppendFila2(APTO_BAIXA,tcb))
						return -1;
					else
						return 0;
					break;
			default:
					return -1;
					break;
		}
}
//*******PEDIU UM RETORNO: COLOQUEI VOID
int removerApto(TCB_t *tcb){
	TCB_t *temp;
 	if(searchTID(APTO_ALTA,tcb->id)){
		if(removeDaFila(APTO_ALTA,temp)){
			return -1;
		}
		else return 0;
	}
	if(searchTID(APTO_MEDIA,tcb->id)){
		if(removeDaFila(APTO_MEDIA,temp)){
			return -1;
		}
		else return 0;
	}
	if(searchTID(APTO_BAIXA,tcb->id)){
		if(removeDaFila(APTO_BAIXA,temp)){
			return -1;
		}
		else return 0;
	}
	return -1;
}

int removeDaFila(PFILA2 fila, TCB_t *tcb)
{	/*Retorna 0 se deleta corretamente o TCB de fila, -1 se houve erro*/
	TCB_t* iterador;
	if(FirstFila2(fila)) //caso falhe a fila ou é vazia ou tem erro.
		return -1;
	while((iterador=GetAtIteratorFila2(fila)) && (NextFila2(fila) == 0)) //itera sobre toda fila
	{
		if(iterador->tid == tcb->tid){
			if(DeleteAtIteratorFila2(fila)) //deleta o elemento, retorna != 0 caso falhe.
          return -1;
			else
          return 0;
		}
	}
	return -1;
}

int procurarApto(TCB_t *tcb){
	switch(tcb->prio){
		case:0
			if(searchTID(APTO_ALTA,tcb->id))
				return -1;
			else
				return 0;
		case:1
			if(searchTID(APTO_ALTA,tcb->id))
				return -1;
			else
				return 0;
		case:2
			if(searchTID(APTO_ALTA,tcb->id))
				return -1;
			else
				return 0;
	  default:
			return -1;
	}
}
