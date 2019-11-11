//    ANDREY VASCONCELOS CHAVES
//    GRR20172630
//    JEAN SANCHUKI
//    GRR20185527

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include "ppos.h"

#define STACKSIZE 32768
#define TAMTIMER 20


int taskCount=0, userTasks = 0, timerCount = TAMTIMER, canSignum;

task_t tMain, *tCurrent, tDispatcher, *tQueue, *tSleeping;


struct itimerval timer;

struct sigaction action ;

unsigned int sysCount;

//prototipos

void dispatcher_body();

task_t* scheduler();

void setTimer();

void print_elem (void *ptr);

// Inicializa o sistema operacional; deve ser chamada no inicio do main()

void ppos_init (){
	
	setvbuf (stdout, 0, _IONBF, 0);
	
	tMain.id = 0;
	tMain.next = NULL;
	tMain.prev = NULL;
//	queue_append((queue_t **) &tQueue, (queue_t *) &tMain);
	tCurrent = &tMain;

	//Dispatcher SO3
	task_create(&tDispatcher, dispatcher_body, "Dispatcher");
	setTimer();
};

void tratador (int signum)
{
	//printf ("Recebi o sinal %d\n\n", signum);
	if (canSignum == 1)
	{
	 	if(signum == 14)
	 	{
	 		#ifdef DEBUG
	 			//printf("activationCounter task %d = %d	  ", tCurrent->id, tCurrent->actCount);
	 		#endif
	 		timerCount--;
	 		#ifdef DEBUG
			//	printf("timerCount == %d\n", timerCount);
			#endif
	 		tCurrent->time++;
	 		sysCount++;
	 	}
	 	if(timerCount==0)
	 		task_yield();
 	}
}

unsigned int systime()
{
	return sysCount;
}

void setTimer()
{
	action.sa_handler = tratador ;
	sigemptyset (&action.sa_mask) ;
	action.sa_flags = 0 ;
  	if (sigaction (SIGALRM, &action, 0) < 0)
  	{
    	perror ("Erro em sigaction: ") ;
    	exit (1) ;
 	}
	// ajusta valores do temporizador
	timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
	timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
	timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
	timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

  	// arma o temporizador ITIMER_REAL (vide man setitimer)
  	if (setitimer (ITIMER_REAL, &timer, 0) < 0)
  	{
    	perror ("Erro em setitimer: ") ;
    	exit (1) ;
  	}

}
// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.

//Argumentos->
// descritor da nova tarefa
// funcao corpo da tarefa
// argumentos para a tarefa
int task_create (task_t *task, void (*start_func)(void *), void *arg)
{
	if (task != NULL)
	{
		//if(task != &tDispatcher)
		//s	userTasks++;
		
		
		taskCount++;
		task->id = taskCount;
		task->next = NULL;
		task->prev = NULL;
		task->prioDin = 0;
		task->prioEst = 0;
		task->time = 0;
		task->actCount = 0;
		task->exited = 0;
		
		getcontext(&task->context);
		
		task->stack = malloc(STACKSIZE);
		
		if(task->stack)
		{
			task->context.uc_stack.ss_sp = task->stack ;
		    task->context.uc_stack.ss_size = STACKSIZE ;
		    task->context.uc_stack.ss_flags = 0 ;
		    task->context.uc_link = 0 ;
		}
		else
		{
			perror ("Erro na criação da pilha: ") ;
      		return -1;
		}
	
		makecontext(&(task->context),(void*)(*start_func),1,arg);
		
		#ifdef DEBUG
			if(task->id == 1){
				printf ("\ntask_create: criou dispatcher \n\n");
			}else{
				printf ("\ntask_create: criou tarefa %d\n\n", task->id);
			}
		#endif

		if (task != &tDispatcher)
			queue_append((queue_t **) &tQueue, (queue_t *) task);

		#ifdef DEBUG
			queue_print ("\nSaida ao criar no create  ", (queue_t*) tQueue, print_elem) ;
		#endif

		userTasks = queue_size((queue_t*)tQueue);	
		return task->id;
	}
	return -1;
};

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode)
{
	//#ifdef DEBUG
	printf ("task %d exit: execution time %d ms, processor time %d ms, %d activations\n",tCurrent->id, systime(), tCurrent->time, tCurrent->actCount);
	//#endif

	
	task_t *aux;
	while(tCurrent->waitingQueue != NULL){
		aux = tCurrent->waitingQueue;
		aux->exitCode = exitCode;
		queue_append((queue_t**)&tQueue, queue_remove((queue_t **)&tCurrent->waitingQueue, (queue_t *) aux));
	}
	userTasks--;
	if(tCurrent != &tDispatcher){
		queue_remove((queue_t **) &tQueue,(queue_t *) tCurrent);
		tCurrent->exited = 1;
	}

	if (tCurrent == &tDispatcher)
		task_switch(&tMain);

	task_switch(&tDispatcher);
};

// alterna a execução para a tarefa indicada
int task_switch (task_t *task)
{
	if(task != NULL)
	{
		task_t *oldTask;
		oldTask = tCurrent;
		tCurrent = task;
		tCurrent->actCount++;

		#ifdef DEBUG
		printf ("\ntask_switch: trocando de contexto %d -> %d no tempo %d\n\n",oldTask->id, tCurrent->id, systime());
		#endif

		swapcontext(&oldTask->context,&task->context);
		return 0;
	}
	return -1;
};

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id ()
{
	return tCurrent->id;	
};

void task_yield()
{
	#ifdef DEBUG
		printf ("\ntask_yield: yield da tarefa %d\n\n", tCurrent->id);
		queue_print ("Saida ao adicionar no yield  ", (queue_t*) tQueue, print_elem) ;
	#endif
	canSignum = 1;
	task_switch(&tDispatcher);
};

void dispatcher_body () // dispatcher é uma tarefa
{
	task_t *next;
	
	while ( userTasks > 0 ){
    	next = scheduler() ;  // scheduler é uma função

    	task_t *aux, *aux2;
    	int i=0;
    	aux = tSleeping;
    	
    	while(tSleeping != NULL && i < queue_size((queue_t *) tSleeping)){
			if(systime() == aux->sleepTime){
				aux2 = aux;
				aux = aux->next;
				queue_append((queue_t**)&tQueue, queue_remove((queue_t**)&tSleeping, (queue_t *) aux2));
			}
			else{
				i++;
				if(tSleeping != NULL)
					aux = aux->next;
			}
		}
    	if (next != NULL){
        // ações antes de lançar a tarefa "next", se houverem
       		timerCount = TAMTIMER;
       		task_switch (next) ; // transfere controle para a tarefa "next"
        // ações após retornar da tarefa "next", se houverem
     	}
   	}
   task_exit(0) ; // encerra a tarefa dispatcher
};

void task_sleep (int t)
{
	tCurrent->sleepTime = systime() + t;
	queue_remove((queue_t**)&tQueue, (queue_t *) tCurrent);
	queue_append((queue_t **)&tSleeping, (queue_t *) tCurrent);
	task_switch(&tDispatcher);
}

task_t *getTarefaImportante()
{
	int maior ;
	task_t *aux, *aux2;
	
	aux = tQueue;
	maior = aux->prioDin;
	aux2 = aux;
	
	for (int i = 0; i < queue_size((queue_t*)tQueue); i++)
	{
		if(aux->prioDin < maior)
		{
			maior = aux->prioDin;
			aux2 = aux;
		}
		aux->prioDin--;
		aux = aux->next;
	}
	
	aux2->prioDin = aux2->prioEst;
	
	return aux2;	
} 

task_t *scheduler()
{
	task_t *aux = NULL;
	
	if(tQueue != NULL)
		aux = getTarefaImportante();
	
	#ifdef DEBUG
		printf("\nO valor do contador de tarefas do usuario eh: %d\n\n", userTasks); 
		queue_print ("\nSaida ao remover no scheduler  ", (queue_t*) tQueue, print_elem); 
	#endif
	
	if (aux == NULL){
		return NULL;
	}
	
	return aux;
};

void print_elem (void *ptr)
{
 	task_t *elem = ptr ;

	if (!elem)
    	return ;

   	elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   	printf ("<%d>", elem->id) ;
   	elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
};

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) 
{
	if ((prio <= 20) && (prio >=-20))
	{
		if (task)
		{
			task->prioEst = prio;
			task->prioDin = task->prioEst;
		}
		else
		{
			tCurrent->prioEst = prio;
			tCurrent->prioDin = tCurrent->prioEst;

		}
	}	
	else
	{
		fprintf(stderr, "Prioridade deve estar entre -20(mais importante) e +20(menos importante");
	}	
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task)
{
	if (task)
		return task->prioDin;
	return tCurrent->prioDin;		
}

int task_join (task_t *task)
{
   	if(task->exited != 1)
   	{	
   		queue_remove((queue_t**) &tQueue, (queue_t *) tCurrent);
	    queue_append((queue_t **)&task->waitingQueue, (queue_t *)tCurrent);
        task_switch(&tDispatcher);
        return(tCurrent->exitCode);
    }
    return(-1);
}

int sem_create (semaphore_t *s, int value)
{
	printf("Criando semaphore\n");
	s->value = value;
	s->valid = true;
	return 0;
}

int sem_down (semaphore_t *s)
{

	if(!s)
		return -1;

	canSignum = 0;
	s->value--;
	if (s->value<0)
	{
	//	printf("Down < 0\n");
		queue_append((queue_t **)&s->queue, queue_remove((queue_t **)&tQueue, (queue_t *)tCurrent));
		task_yield();
	}
	canSignum = 1;
	if (s->valid == true)
		return 0;
	return -1;
}

int sem_up (semaphore_t *s)
{
	if(!s)
		return -1;

	canSignum = 0;
	s->value++;
	if (s->value <= 0 && s->queue != NULL)
	{
		queue_append((queue_t **)&tQueue, queue_remove((queue_t **)&s->queue, (queue_t *)s->queue));
	}
	canSignum = 1;
	if (s->valid == true)
		return 0;
	return -1;
}

int sem_destroy (semaphore_t *s)
{
	canSignum = 0;
	if (s->valid == false)
	{
		canSignum = 1;
		return -1;
	}
	while(s->queue != NULL)
	{
		queue_append((queue_t**)&tQueue, queue_remove((queue_t **)&s->queue, (queue_t *)s->queue));
	}
	s->valid = false;
	canSignum = 1;
	return 0;
}

void produtor()
{

}

int mqueue_create (mqueue_t *queue, int max, int size)
{
	if ((max > 0) && (queue != NULL) && size > 0)
	{	

		queue->buffer = (mqueue_t*) malloc (max*size);
		queue->msg = queue->buffer;
		queue->nextMsg = queue->buffer;
		queue->maxsize = max;
		queue->tamMsg = size;
		queue->numMensagens = 0;
		queue->semFilaMensagens = malloc (sizeof(semaphore_t));
		queue->semNumMensagens = malloc (sizeof(semaphore_t));
		queue->semBuffer = malloc (sizeof(semaphore_t));
		sem_create(queue->semFilaMensagens, queue->maxsize);
		sem_create(queue->semNumMensagens, queue->numMensagens);
		sem_create(queue->semBuffer, 1);
		return 0;
	}
	return -1;
}

int mqueue_send (mqueue_t *queue, void *msg)
{
	if (queue)
	{
		//Verifica se a fila foi destruida
		if(queue->semBuffer != NULL)
		{
			sem_down(queue->semBuffer);
			sem_up(queue->semBuffer);
			memcpy(queue->nextMsg, msg, queue->tamMsg);

			if ((queue->nextMsg + queue->tamMsg) == (queue->buffer + (queue->maxsize * queue->tamMsg)))
				queue->nextMsg = queue->buffer;
			else 
				queue->nextMsg += queue->tamMsg;

			queue->numMensagens++;
			sem_up(queue->semBuffer);
			sem_up(queue->semNumMensagens);
			return 0;
		}	
		
	}
	return -1;
}

int mqueue_recv (mqueue_t *queue, void *msg) 
{
	if (queue)
	{
		//Verifica se a fila foi destruida
		if(queue->semBuffer != NULL)
		{
			sem_down(queue->semNumMensagens);
			sem_down(queue->semBuffer);
			memcpy(msg, queue->msg, queue->tamMsg);

			if ((queue->msg + queue->tamMsg) == (queue->buffer + (queue->maxsize * queue->tamMsg))) 
				queue->msg = queue->buffer;
			else 
				queue->msg += queue->tamMsg;

			sem_up(queue->semBuffer);
			sem_up(queue->semFilaMensagens);

			return 0;
		}	
	}
	return -1;
}

int mqueue_destroy (mqueue_t *queue) 
{
	if (queue)
	{
		sem_destroy(queue->semFilaMensagens);
		sem_destroy(queue->semBuffer);
		sem_destroy(queue->semNumMensagens);

		queue->semBuffer = NULL;
		queue->semNumMensagens = NULL;
		queue->semFilaMensagens = NULL;

		free(queue->buffer);
		queue->buffer = NULL;

		return 0;
	}
	return -1;
}

int mqueue_msgs (mqueue_t *queue)
{
	if(queue)
		return queue->semNumMensagens->value;
	return -1;	
}
