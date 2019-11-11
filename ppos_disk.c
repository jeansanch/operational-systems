#include "ppos.h"
#include "ppos_disk.h"
#include "hard_disk.h"
#include <signal.h>
#include "queue.h"

#include <stdio.h>

extern task_t *tCurrent;
extern task_t tDispatcher;
extern queue_t *tQueue;

typedef struct task_ES {
	struct task_ES *prev, *next;
	int command;
	int block;
	void *buffer;
	task_t *owner;
} task_ES;

struct sigaction diskHandler; 

disk_t harddisk;
queue_t *queue_disk;

static task_t disk_manager; // disk driver

void print_elem2 (void *ptr)
{
 	task_t *elem = ptr ;

	if (!elem)
    	return ;

   	elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   	printf ("<%d>", elem->id) ;
   	elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
};

int disk_mgr_init(int *num_blocks, int *block_size) {
	task_create(&disk_manager, diskDriverBody, "");
	queue_append((queue_t **)&tQueue, (queue_t *) &disk_manager);
	diskHandler.sa_handler = disk_handler;
	if (disk_cmd(DISK_CMD_INIT, 0, 0) < 0)
		return -1;
	*num_blocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
	*block_size = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
	
	if (*num_blocks < 0 || *block_size < 0 || sem_create(&(harddisk.access), 1) == -1)
		return -1;
	return 0;
}



void diskDriverBody() {
	printf("chegou aqui\n");
	while (true) {
 // obtem o semaforo de acesso ao disco
		sem_up(&(harddisk.access));

 // se foi acordado devido a um sinal do disco
		if (harddisk.signal == 1) {
			// acorda a tarefa cujo pedido foi atendido
			harddisk.signal = 0;
			task_ES *task = (task_ES*)queue_remove(&(queue_disk), queue_disk);
			queue_append((queue_t **)&tQueue, (queue_t *) task->owner);
		}
 // se o disco estiver livre e houver pedidos de e/s na fila		
		if (disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE && queue_size((queue_t *)queue_disk) > 0) {
			task_ES *task =  (task_ES*)queue_remove(&(queue_disk), queue_disk); // FCFS
			disk_cmd(task->command, task->block, task->buffer);
		}
 // libera o semaforo
		sem_up(&(harddisk.access));
 // suspende a tarefa corrente
		task_switch(&tDispatcher);
	}
}


int disk_block_read(int block, void *buffer) {
	sem_down(&(harddisk.access));

	task_ES task;
	task.block = block;
	task.buffer = buffer;
	task.command = DISK_CMD_READ;
	task.owner = tCurrent;

	queue_append(&(queue_disk), (queue_t *) &task);

	queue_append(&(tQueue), (queue_t *) &disk_manager);

	sem_up(&(harddisk.access));

	task_switch(&tDispatcher);

	return 0;
}


void disk_handler(int signal_number)
{
	sem_down(&(harddisk.access));
	harddisk.signal = 1;
	queue_append(&(tQueue), (queue_t *)&disk_manager);
	sem_up(&(harddisk.access));
}

int disk_block_write(int block, void *buffer) {
	sem_down(&(harddisk.access));

	task_ES task;
	task.block = block;
	task.buffer = buffer;
	task.command = DISK_CMD_WRITE;
	task.owner = tCurrent;

	queue_append((queue_t **)&queue_disk, (queue_t *) &task);

	queue_append((queue_t**)&tQueue, (queue_t *) &disk_manager);

	sem_up(&(harddisk.access));

	task_switch(&tDispatcher);

	return 0;
}

