#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila
void queue_append (queue_t **queue, queue_t *elem)
{
    if (!queue)
    {
        fprintf(stderr, "Impossivel inserir em fila nula\n");
        return;
    }
    if (elem == NULL)
    {
        fprintf(stderr, "Elemento nao existe\n");
        return;
    }
    if (elem->prev != NULL)
    {
       // fprintf(stderr, "Elemento pertence a outra fila\n");
        return;
    }
       
    queue_t *aux;
    if ((*queue))
    {
        aux = (*queue)->prev;
        (*queue)->prev = elem;
        aux->next = elem;
        elem->prev = aux;
        elem->next = (*queue);
    }
    else
    {
        (*queue) = elem;
        (*queue)->next = elem;
        (*queue)->prev = elem;
    }
    

} 

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: apontador para o elemento removido, ou NULL se erro
queue_t *queue_remove (queue_t **queue, queue_t *elem)
{    
    
    if (queue == NULL)
    {
        fprintf(stderr, "Impossivel remover de fila nula\n");
        return NULL;
    }
    if (*queue == NULL)
    {
        fprintf(stderr, "Fila vazia\n");
        return NULL;
    }
    if (elem == NULL)
    {
        fprintf(stderr, "Elemento nao existe\n");
        return NULL;
    }

    if (*queue == (*queue)->next && *queue == elem)
    {
        *queue = NULL;
        elem->next = NULL;
        elem->prev = NULL;
        return elem;
    }
    if (*queue == elem)
    {
        (*queue)->next->prev = (*queue)->prev;
        (*queue)->prev->next = (*queue)->next;
        *queue = (*queue)->next;

        elem->next = NULL;
        elem->prev = NULL;
        return elem;        
    } 

    queue_t *aux = *queue;
    *queue = (*queue)->next;
    while(*queue != aux)
    {
        if (*queue == elem)
        {
            (*queue)->next->prev = (*queue)->prev;
            (*queue)->prev->next = (*queue)->next;
            *queue = aux;

            elem->next = NULL;
            elem->prev = NULL;
            return elem;        
        }    

        *queue = (*queue)->next;
    }
    return NULL;
}
//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila
int queue_size (queue_t *queue)
{
    int count = 1;

    if(queue == NULL)
    {
        return 0;
    }
    else
    {
        queue_t *aux;
        aux = queue;
        while (aux->next != queue)
        {
             count++;
             aux = aux->next;
        }
    }
    return count;     
    
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca.
//
// Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir
void queue_print (char *name, queue_t *queue, void print_elem (void*) )
{
    printf("%s", name);
    printf(": [");
    for (int i = 0; i < queue_size(queue); i++)
    {
        print_elem(queue);
        printf(" ");
        queue = queue->next;
    }
    printf("]");
    printf("\n");
}

