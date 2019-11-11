// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Vers�o 1.1 -- Julho de 2016
//
// interface de acesso ao disco r�gido

#ifndef __PPOS_DISK__
#define __PPOS_DISK__

#include "ppos_data.h"

// structura de dados que representa o disco para o SO
typedef struct
{
    semaphore_t access; 
    int signal;
} disk_t ;

// inicializacao do gerenciamento de disco
// retorna -1 em erro ou 0 em sucesso
// num_blocks: tamanho do disco, em blocos
// block_size: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *num_blocks, int *block_size) ;

// leitura de um bloco, do disco para o buffer indicado
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer indicado para o disco
int disk_block_write (int block, void *buffer) ;

void diskDriverBody();

void disk_handler(int signal_number);

#endif

