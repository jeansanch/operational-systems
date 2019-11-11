//    ANDREY VASCONCELOS CHAVES
//    GRR20172630
//    JEAN SANCHUKI
//    GRR20185527
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>   // biblioteca POSIX de trocas de contexto
#include "queue.h"    // biblioteca de filas genéricas
#include <stdbool.h>
// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
   struct task_t *prev, *next ;   // ponteiros para usar em filas
   struct task_t *waitingQueue;
   int id ;       // identificador da tarefa
   ucontext_t context ;     // contexto armazenado da tarefa
   void *stack ;  // aponta para a pilha da tarefa
   int prioDin;     
   int prioEst;
   int time;
   int actCount;
   int exitCode;
   int exited;
   unsigned int sleepTime;
   // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct semaphore_t
{
  struct task_t *queue;
  int value;
  bool valid;
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens

typedef struct mqueue_t
{
  struct mqueue_t *prev, *next;
  int first, end;
  int maxsize, numMensagens, tamMsg;
  void *msg, *buffer, *nextMsg;

  semaphore_t *semFilaMensagens, *semNumMensagens, *semBuffer;
  
  // preencher quando necessário
} mqueue_t ;

#endif

