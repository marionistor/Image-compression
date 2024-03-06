#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdio.h>
#include <stdlib.h>

// coada folosita pentru parcurgerea pe nivel a arborelui
typedef struct node {
    quad_tree *tree_node;
    struct node *next;
} node;

typedef struct queue {
    node *front;
    node *rear;
    long size;
} queue;

// functie pentru alocarea memoriei cozii
void init_queue(queue **q)
{
    *q = malloc(sizeof(queue));
    (*q)->front = (*q)->rear = NULL;
    (*q)->size = 0;
}

// functie pentru a verifica daca este goala coada
int is_empty(queue *q)
{
    if(q->front == NULL)
        return 1;
    return 0;    
}

// functie pentru adaugarea unui element in coada
void enqueue(queue *q, quad_tree *tree_node)
{
    node *new_node = malloc(sizeof(node));
    new_node->tree_node = tree_node;
    new_node->next = NULL;
    if(is_empty(q))
    {
        q->front = q->rear = new_node;
    }
    else
    {
        q->rear->next = new_node;
        q->rear = new_node;
    }
    q->size++;
}

// functie pentru eliminarea unui element din coada
quad_tree *dequeue(queue *q)
{
    node *aux = q->front;
    quad_tree *tree_node = aux->tree_node;
    q->front = q->front->next;
    q->size--;
    if(q->size == 0)
    {
        q->rear = NULL;
    }
    free(aux);
    return tree_node;   
}

#endif
