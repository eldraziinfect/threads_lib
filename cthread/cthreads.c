#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/cthread.h"
#include "../include/support.h"
#include "../include/cdata.h"

typedef struct jcb
{

} JCB_t;
/* Funções auxiliares */
TCB_t* searchTID(PFILA2 pFila, int tid);
TCB_t* pickHighestPriority();
int sortThreads(csem_t *sem);
int escalonador(void);
int despachante(TCB_t *proximo);
int init_threads(int prio);
/* Variáveis Globais */
int tid = 0;
int are_init_threads =0;
int t_count = 0; // quantidade de threads


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

/*
int ccreate (void *(*start) (void *), void *arg, int prio)
{
	if(are_init_threads ==0) init_thread();

	TCB_t* thread = (TCB_t*) malloc (sizeof(TCB_t));

	thread->prio = prio;
	thread->state = PROCST_CRIACAO;
	thread->tid = t_count;
	t_count++;

	if (getcontext(&thread->context) == -1)
	    return -1;
*/
//Inicializa contexto
/*
(thread->context).uc_link = &returncontext;
(thread->context).uc_stack.ss_sp = malloc (TAM_MEM * sizeof(char));
(thread->context).uc_stack.ss_size = TAM_MEM;
makecontext(&(thread->context), (void(*)())start,1,arg);
*/

//ao final do processo de criação, a thread deverá ser inserida na fila dos aptos
//	if(EXECUTANDO->prio < thread->prio){
//	despachante(thread);
//}

//if (adicionarApto(novo_tcb))
//   return -1;

//	return tid;
//}

//////////////////////////////******************************
// inserirApto(EXECUTANDO) em qual das filas dos aptos vai inserir???? ja q temos 3
// Cedência voluntária:
int cyield(void)
{
    EXECUTANDO->state = PROCST_APTO; //Passa de executando pra apto.
    if(inserirApto(EXECUTANDO)) // retorna 0 caso tenha obtido sucesso, igual ao AppendFila2
        return -1;
    else
    {
        escalonador();
        return 0;
    }
}

// Setar prioridade:
int csetprio(int tid, int prio)
{
    //Aparentemente é pra deixar tid em null em 2018/2, então o csetprio funciona apenas na thread corrente.
    if(prio >= 0 && prio <= 2)
    {
        if(EXECUTANDO->prio > prio)
        {
            // Executar caso o processo tenha baixado a prioridade, executar o escalonador
            return 0;
        }
        else
        {
            EXECUTANDO->prio = prio;
            return 0;
        }
    }
    else //valor de prioridade é inválido, deve estar entre [0,2].
        return -1;
}


//**** ALGUEM TAVA COMEÇANDO
// eu tava
/
int cjoin(int tid){

/*
* Verificar a existência da thread
* Verificar se a thread já está sendo esperada
*/

if((procurarApto) && (searchTID(BLOQUEADO,tid) == NULL))
	return -1;


/*
int cjoin(int tid){

  if(init_threads == 0)  init_cthreads(); // threads inicializadas

  if(tid == 0){
	printf("ERRO: erro no TID da thread main.\n");
	return -1;
  }

   if((find_thread(tid, APTO_ALTA) != 0) &&
       (find_thread(tid, APTO_MEDIA) != 0) &&
       (find_thread(tid, APTO_BAIXA) != 0)){
    	if(find_thread(tid, BLOQUEADO) != 0){
		printf("ERRO: Não encontrou a thread\n");
    		return -1;
    }

   TCB_t *thread = EXECUTANDO;

  / não mudei a struct pq não sabía como q vai ser/
  JCB_t *jcb = malloc(sizeof(JCB_t));
  jcb->tid = tid;
  jcb->thread = thread;

   if(AppendFila2(&filaJCB, (void*) jcb) != 0){
    printf("ERRO: erro ao inserir na fila jcb\n\n");
  }

  thread->state = PROCST_BLOQ;
  if(AppendFila2(&filaBloqueados, (void*) thread) != 0){
    printf("ERRO: não pode inserir na fila de bloqueados\n\n");
  }

  EXECUTANDO = 0;
  swapcontext(&thread->context, &r_context);

  return 0;
 }

*/
// Semáforo
int csem_init(csem_t *sem, int count)
{
    /*
    * Exclusão mútua:
    * Dois ou mais processos não podem estar simultaneamente na seção crítica
    */
    PFILA2 SEMAFOROS = (PFILA2)malloc(sizeof(FILA2));
    if(CreateFila2(SEMAFOROS))
        return -1; //falha na criação da fila para semáforos.
    if(count < 0)
        return -1;
    sem->fila = SEMAFOROS;
    sem->count = count;
    if(count > 0)
        printf("AVISO: Para haver exclusão mútua a quantidade de recursos deve ser 1.\nA quantidade informada foi de: %d\nIsso pode resultar em erros!", count);
    return 0;

}
// Semáforo
int cwait(csem_t *sem)
{
    sem->count--;

    if(sem->count < 0)
    {
        TCB_t *thread;
        thread = EXECUTANDO;
        thread->state = PROCST_BLOQ;

        AppendFila2(sem->fila, (void *) thread); //Por que o void*?
        //AppendFila2(&filaBloqueados, (void *) thread); //Não precisa Inserir na fila de bloqueados, apenas mudar o estado do EXECUTANDO, o escalonador faz o swap.

        //EXECUTANDO = 0; //o correto seria chamar o escalonador.
        escalonador();
        //swapcontext(BLOQUEADO, &r_context); //vou deixar assim por enquanto.
        return 0;
    }
    return 0;
}
// Semáforo
int csignal(csem_t *sem)
{
    //para cada chamadada primitiva a variável count deve ser decrementada
    // de uma unidade
    /*
    * TODO: Percorrer a fila de threads do semáforo para escolher a thread de maior prioridade.
    */
    sem->count += 1;
    if(sem->count <= 0)
    {
        if(sortThreads(sem)) //Sorting para escolher a thread de maior prioridade(sendo 0 a maior).
            return -1;
        TCB_t *temp = GetAtIteratorFila2(sem->fila);
        temp->state = PROCST_APTO; //a thread escolhida vai para o apto.
        if(adicionarApto(temp)) // Adicionada ao apto
            return -1;
        if(removeDaFila(BLOQUEADO,temp)) // Removida do bloqueado
            return -1;
        if(DeleteAtIteratorFila2(sem->fila)) //A lista de threads bloqueadas pelo semáforo é decrescida.
            return -1;
    }
    return 0;
}

int cidentify (char *name, int size)
{
    ;
    char nomes[] = "Carine Bertagnolli Bathaglini - 00274715\nGabriel Pakulski da Silva - 00274701\nLuiz Miguel Kruger - 00228271\n";
    int i,flag = 0;
    for(i = 0; i < size; i++)
    {
        if(nomes[i] == '\0')
        {
            flag = 1;
            break;
        }
    }
    if(flag == 0) return -1;
    for(i = 0; i < size && nomes[i] != '\0'; i++)
        name[i] = nomes[i];

    name[i] = '\0';
    return 0;
}


/********************* FUNÇÕES AUXILIARES *********************/
int sortThreads(csem_t *sem)  //É pra ser um bubblesort nas threads do semáforo;
{
    int i,n = 0;
    if(FirstFila2(sem->fila))
        return -1;
    while(GetAtIteratorFila2(sem->fila))
    {
        n++;
        NextFila2(sem->fila);
    }
    TCB_t *temp = (TCB_t*)malloc(sizeof(TCB_t));
    for(i = 1; i < n; i++)
    {
        if(FirstFila2(sem->fila))
            return -1;
        while(GetAtNextIteratorFila2(sem->fila))
        {
            TCB_t *atual = GetAtIteratorFila2(sem->fila);
            TCB_t *proximo = GetAtNextIteratorFila2(sem->fila);
            if(atual->prio > proximo->prio)
            {
                temp = proximo;
                if(InsertBeforeIteratorFila2(sem->fila,temp))
                    return -1;
                if(NextFila2(sem->fila))
                    return -1;
                if(DeleteAtIteratorFila2(sem->fila))
                    return -1;
            }
            else
            {
                if(NextFila2(sem->fila))
                    return -1;
            }
        }
    }
    if(FirstFila2(sem->fila)) //retorna o apontador para o primeiro elemento.
        return -1;
    return 0;
}


int init_threads(int prio)
{
    //caso em que as filas falharam em sua criação
    if(CreateFila2(APTO_ALTA))
    {
        printf("ERRO: falha ao criar fila dos aptos de prioridade alta\n");
        return -1;
    }
    if(CreateFila2(APTO_MEDIA))
    {
        printf("ERRO: falha ao criar fila dos aptos de prioridade média\n");
        return -1;
    }
    if(CreateFila2(APTO_BAIXA))
    {
        printf("ERRO: falha ao criar fila dos aptos de prioridade baixa\n");
        return -1;
    }
    if(CreateFila2(BLOQUEADO))
    {
        printf("ERRO: falha ao criar fila dos bloqueados\n");
        return -1;
    }
    if(CreateFila2(F_EXECUTANDO))
    {
        printf("ERRO: falha ao criar fila join\n");
        return -1;
    }

    EXECUTANDO = (TCB_t*) malloc(sizeof(TCB_t));

    EXECUTANDO->prio = prio; // prioridade da thread é passada no parâmetro
    EXECUTANDO->tid = tid;
    EXECUTANDO->state = PROCST_APTO; // a thread está apta

//	EXECUTANDO = new_thread;

    /** ??????????????????????????????????????????????????????????????????????????????????????
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
    	lockReturn = 1; */

    are_init_threads = 1;
}


int find_thread(int tid, PFILA2 fila)
{

    TCB_t *thread = malloc(sizeof(TCB_t));
    if(FirstFila2(fila) != 0)
    {
        printf("ERRO: fila vazia\n");
        return -1;
    }

    do
    {
        if(fila->it == 0)  break;
        thread = (TCB_t *)GetAtIteratorFila2(fila);
        if(thread->tid == tid) return 0;
    }
    while(NextFila2(fila) == 0);

    return -1;
}


TCB_t* searchTID(PFILA2 fila, int tid)
/*Procura numa fila se existe o processo de tid e retorna | um ponteiro para o TCB caso positivo																											 | NULL caso contrário*/
{
    TCB_t* tcb; //// cadê o malloc?
    if (FirstFila2(fila))
    {
        print("ERRO: a fila está vazia\n");
        return NULL;
    }
    while(tcb = (TCB_t*)GetAtIteratorFila2(fila))
    {
        if(tcb->tid == tid) return tcb;
        else if (NextFila2(fila)) return NULL;
        return NULL;
    }
}

/*
* 	OBJETIVO: inserção da thread em alguma das filas de apto, a escolha por determinada fila
*		  se dá através de sua prioridade.
*/
int inserirApto(TCB_t* thread)
{
    switch(thread->prio)
    {
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

int escalonador()
{
    /*
    * Implementação de um escalonador preemptivo por prioridade
    */
    TCB_t *proximo = pickHighestPriority();
    if(proximo == NULL)
    {
        printf("ERRO: A fila de aptos se encontra vazia!\n");
        return -1; // -1 indica que a fila de aptos está vazia
    }
    if(proximo->prio > EXECUTANDO->prio)
    {
        despachante(proximo);
        return 0; //retorna 0 caso a troca de contexto tenha ocorrido.
    }
    return 1; //retorna 1 caso não tenha ocorrido troca de contexto.
}

TCB_t* pickHighestPriority()
{
    TCB_t *escolhido;
    if(FirstFila2(APTO_ALTA))  // Verdadeiro se não tem nenhum elemento de prioridade alta
    {
        if(FirstFila2(APTO_MEDIA))  // Verdadeiro se não tem nenhum elemento de prioridade alta ou média:
        {
            if(FirstFila2(APTO_BAIXA))  //Verdadeiro se não tem nenhum elemento em nenhuma fila (alta, media e baixa)
            {
                return NULL;
            }
            else  //Existe pelo menos um TCB de prioridade baixa.
            {
                escolhido = GetAtIteratorFila2(APTO_BAIXA);
            }
        }
        else //Existe pelo menos um TCB de prioridade media.
        {
            escolhido = GetAtIteratorFila2(APTO_MEDIA);
        }
    }
    else //Existe pelo menos um TCB de prioridade alta.
    {
        escolhido = GetAtIteratorFila2(APTO_ALTA);
    }
    return escolhido;
}

//*******PEDIU UM RETORNO: COLOQUEI VOID
int despachante(TCB_t *proximo)
{
    int estado = EXECUTANDO->state; //o próximo estado do executando define qual tratamento ele receberá.
    TCB_t *temp;
    switch(estado)
    {
    case PROCST_APTO: //É necessário colocar o processo na fila de aptos.
        proximo->state = PROCST_EXEC;
        temp = EXECUTANDO;
        EXECUTANDO = proximo;
        if(removerApto(proximo))
            return -1;
        swapcontext(&(temp->context),&(EXECUTANDO->context));
        return 0;
        break;
    case PROCST_BLOQ: //é necessário colocar o processo na fila de bloqueados.
        if(AppendFila2(BLOQUEADO,EXECUTANDO))
            return -1;
        temp = EXECUTANDO;
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


int adicionarApto (TCB_t *tcb)
{
    /* Retorna 0 caso tenha funcionado e -1 cc.
    */
    switch(tcb->prio)
    {
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

int removerApto(TCB_t *tcb)
{
    TCB_t *temp;
    if(searchTID(APTO_ALTA,tcb->tid))
    {
        if(removeDaFila(APTO_ALTA,temp))
        {
            return -1;
        }
        else return 0;
    }
    if(searchTID(APTO_MEDIA,tcb->tid))
    {
        if(removeDaFila(APTO_MEDIA,temp))
        {
            return -1;
        }
        else return 0;
    }
    if(searchTID(APTO_BAIXA,tcb->tid))
    {
        if(removeDaFila(APTO_BAIXA,temp))
        {
            return -1;
        }
        else return 0;
    }
    return -1;
}

int removeDaFila(PFILA2 fila, TCB_t *tcb)
{
    /*Retorna 0 se deleta corretamente o TCB de fila, -1 se houve erro*/
    TCB_t* iterador;
    if(FirstFila2(fila)) //caso falhe a fila ou é vazia ou tem erro.
        return -1;
    while((iterador=GetAtIteratorFila2(fila)) && (NextFila2(fila) == 0))  //itera sobre toda fila
    {
        if(iterador->tid == tcb->tid)
        {
            if(DeleteAtIteratorFila2(fila)) //deleta o elemento, retorna != 0 caso falhe.
                return -1;
            else
                return 0;
        }
    }
    return -1;
}

int remove_thread(int tid, PFILA2 fila)
{
    TCB_t *thread;
    FirstFila2(fila);

    do
    {
        if(fila->it == 0)
        {
            break;
        }

        thread = (TCB_t *)GetAtIteratorFila2(fila);

        if(thread->tid == tid)
        {
            DeleteAtIteratorFila2(fila);
            return 0;
        }
    }
    while(NextFila2(fila) == 0);
    return -1;
}


int procurarApto(TCB_t *tcb)
{
    switch(tcb->prio)
    {
    case 0:
        if(searchTID(APTO_ALTA,tcb->tid))
            return -1;
        else
            return 0;
    case 1:
        if(searchTID(APTO_ALTA,tcb->tid))
            return -1;
        else
            return 0;
    case 2:
        if(searchTID(APTO_ALTA,tcb->tid))
            return -1;
        else
            return 0;
    default:
        return -1;
    }
    return 0;
}
