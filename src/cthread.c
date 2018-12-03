#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/cthread.h"
#include "../include/support.h"
#include "../include/cdata.h"

#define TAM_MEM 1024
/* Funções auxiliares */
int init_threads(int prio);
int removerApto(TCB_t *tcb);
int adicionarApto (TCB_t *tcb);
TCB_t* searchTID(PFILA2 pFila, int tid);
TCB_t* pickHighestPriority();
int sortThreads(csem_t *sem);
int escalonador(void);
int despachante(TCB_t *proximo);
TCB_t* searchData(int tid);
int remove_thread(int tid, PFILA2 fila); //via tid
int removeDaFila(PFILA2 fila, TCB_t *tcb); //via ponteiro para tcb.

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


int ccreate (void *(*start) (void *), void *arg, int prio)
{

    if(are_init_threads == 0) //Ou seja, é a thread main
        init_threads(0); //inicia as filas, aloca um tcb para main - prioridade da main = 0
    t_count++;
    TCB_t* thread = (TCB_t*)malloc(sizeof(TCB_t));
    thread->prio = prio;
    thread->state = PROCST_CRIACAO;
    thread->tid = t_count;
    if (getcontext(&(thread->context)) == -1)
        return -1;
//Inicializa contexto

    (thread->context).uc_link = &r_context;
    (thread->context).uc_stack.ss_sp = malloc (TAM_MEM * sizeof(char));
    (thread->context).uc_stack.ss_size = TAM_MEM;
    thread->state = PROCST_APTO;

    makecontext(&(thread->context), (void(*)())start,1,arg);
//ao final do processo de criação, a thread deverá ser inserida na fila dos aptos
    if(EXECUTANDO->prio > prio)
    {
        if (adicionarApto(thread))
            return -1;
        printf("PREEMPCAO NECESSARIA, ESCALONADOR INVOCADO\n");
        escalonador();
    }
    if (adicionarApto(thread))
        return -1;
    printf("\n------------------------------------------------\n");
    printf("Contexto criado para thread %d\n",thread->tid);
    printf("Nao ocorreu preempcao\n");
    printf("Thread Adicionada ao apto\n");
    printf("TID: %d\n",t_count);
    printf("------------------------------------------------\n\n");
    return t_count;
}

// Cedência voluntária:
int cyield(void)
{
    EXECUTANDO->state = PROCST_APTO; //Passa de executando pra apto.
    if(adicionarApto(EXECUTANDO)) // retorna 0 caso tenha obtido sucesso, igual ao AppendFila2
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
            EXECUTANDO->state = PROCST_APTO;
            if(adicionarApto(EXECUTANDO))
                return -1;
            escalonador();
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


int cjoin(int tid)
{
    /*
    * Verificar a existência da thread
    * Verificar se a thread já está sendo esperada
    */
    printf("CJOIN invocada, esperando thread %d\n",tid);
    if(searchData(tid)!= NULL || (searchTID(BLOQUEADO,tid)==NULL &&
                                  searchTID(APTO_ALTA,tid)==NULL &&
                                  searchTID(APTO_MEDIA,tid)==NULL &&
                                  searchTID(APTO_BAIXA,tid)==NULL))// thread já está bloqueada ou não existe
    {
        printf("Não encontrei a thread %d em fila ALGUMA !!!!\n", tid);
        return -1;
    }
    else
    {
        printf("Thread encontrada!\nThread %d movida para BLOQUEADO\n",EXECUTANDO->tid);
        EXECUTANDO->data = (void*)tid;
        EXECUTANDO->state = PROCST_BLOQ;
        escalonador();
        return 0;
    }
    return -2; //só cai aqui em caso de erro por radiação
}


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

        AppendFila2(sem->fila, (void *) thread);

        escalonador();
        return 0;
    }
    return 0;
}
// Semáforo
int csignal(csem_t *sem)
{
    //para cada chamadada primitiva a variável count deve ser decrementada
    // de uma unidade
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
        printf("ERRO: falha ao criar fila executando\n");
        return -1;
    }
    TCB_t* fthread = (TCB_t*) malloc(sizeof(TCB_t *));

    fthread->prio = prio; // prioridade da thread é passada no parâmetro
    fthread->tid = 0; // main recebe o tid 0;
    fthread->state = PROCST_EXEC; // a thread está executando
    getcontext(&(fthread->context));
    EXECUTANDO = fthread;
    getcontext(&(r_context)); //seta o endereço de retorno.
    are_init_threads = 1;
    return 0;
}


int find_thread(int tid, PFILA2 fila)
{
    TCB_t *thread = malloc(sizeof(TCB_t));
    if(FirstFila2(fila) != 0)
    {
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

TCB_t* searchDataAux(PFILA2 fila, int tid)
{
    TCB_t* thread = (TCB_t*)malloc(sizeof(TCB_t));

    if(FirstFila2(fila) != 0)
    {
        return NULL;
    }
    do
    {
        if(fila->it == 0)
            break;
        thread = (TCB_t *)GetAtIteratorFila2(fila);
        if((int)thread->data == tid)
        {
            return thread;
        }
    }
    while(NextFila2(fila) == 0);

    return NULL;
}

TCB_t* searchData(int tid)
{
    TCB_t* thread = (TCB_t*)malloc(sizeof(TCB_t));
    thread = searchDataAux(APTO_ALTA,tid);
    if(thread) return thread;
    thread = searchDataAux(APTO_MEDIA,tid);
    if(thread) return thread;
    thread = searchDataAux(APTO_BAIXA,tid);
    if(thread) return thread;
    thread = searchDataAux(BLOQUEADO,tid);
    if(thread) return thread;

    return NULL;
}


TCB_t* searchTID(PFILA2 fila, int tid)
/*Procura numa fila se existe o processo de tid e retorna
	| um ponteiro para o TCB caso positivo
	| NULL caso contrário*/
{
    if (FirstFila2(fila))
    {
        return NULL;
    }
    TCB_t* tcb = (TCB_t*)GetAtIteratorFila2(fila);
    while(tcb)
    {
        if(tcb->tid == tid){
            return tcb;
        }
        else if (NextFila2(fila))
            return NULL;
        tcb = (TCB_t*)GetAtIteratorFila2(fila);
    }
    return NULL;
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
        printf("ERRO! Prioridade de processo invalida!\n");
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
        return -1; // -1 indica que a fila de aptos está vazia
    }
    if(proximo->prio > EXECUTANDO->prio || EXECUTANDO->state == PROCST_BLOQ)
    {
        despachante(proximo);
        return 0; //retorna 0 caso a troca de contexto tenha ocorrido.
    }
    return 1; //retorna 1 caso não tenha ocorrido troca de contexto.
}

TCB_t* pickHighestPriority()
{
    TCB_t *escolhido = (TCB_t*)malloc(sizeof(TCB_t));
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
        proximo->state = PROCST_EXEC;
        temp = EXECUTANDO;
        EXECUTANDO = proximo;
        if(removerApto(proximo))
            return -1;

        setcontext(&EXECUTANDO->context);
        if(swapcontext(&(temp->context),&(EXECUTANDO->context)) == -1)
            return -1;
        return 0;
        break;
    case PROCST_TERMINO: //o processo foi terminado: desalocar o PCB.
        /*
        * Nesse caso, varrer as 5 listas pra ver se não tinha ninguém
        * esperando por ele, se for o caso, transferir do bloqueado -> apto.
        */
        temp = searchData(EXECUTANDO->tid); //verifica se existia alguém esperando pelo término do processo em execução.
        if(temp)
        {
            temp->state = PROCST_APTO;
            if(remove_thread(temp->tid,BLOQUEADO)) //remove do bloqueado.
                return -1;
            if(adicionarApto(temp)) //adiciona em apto
                return -1;
        }
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
    return -1;
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
    TCB_t *temp = searchTID(APTO_ALTA,tcb->tid);
    if(temp)
    {
        if(removeDaFila(APTO_ALTA,temp))
        {
            return -1;
        }
        else return 0;
    }
    temp = searchTID(APTO_MEDIA,tcb->tid);
    if(temp)
    {
        if(removeDaFila(APTO_MEDIA,temp))
        {
            return -1;
        }
        else return 0;
    }
    temp = searchTID(APTO_BAIXA,tcb->tid);
    if(temp)
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
        if(searchTID(APTO_MEDIA,tcb->tid))
            return -1;
        else
            return 0;
    case 2:
        if(searchTID(APTO_BAIXA,tcb->tid))
            return -1;
        else
            return 0;
    default:
        return -1;
    }
    return 0;
}
