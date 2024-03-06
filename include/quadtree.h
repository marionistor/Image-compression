/* Nistor Ioan-Mario-311CC */

#ifndef QUADTREE_H_
#define QUADTREE_H_

#include <stdio.h>
#include <stdlib.h>

// structura unui pixel din imagine
typedef struct pixel {
    u_chr R;
    u_chr G;
    u_chr B;
} pixel;

/* pentru arborele cuaternar retin in campul pix_node 0 daca nodul nu este
   frunza respectiv valorile medii red, green, blue daca este frunza si 
   nivelul la care se afla, cu ajutorul caruia pot determina numarul de 
   niveluri din arbore */
typedef struct quad_tree {
    pixel pix_node;
    u_int level;
    struct quad_tree *up_left, *up_right, *down_right, *down_left;
} quad_tree;

// functie pentru alocare de memorie si initializare a unui nod din arbore
void init_node(quad_tree **node)
{
    *node = malloc(sizeof(quad_tree));
    (*node)->pix_node.R = 0;
    (*node)->pix_node.G = 0;
    (*node)->pix_node.B = 0;
    (*node)->up_left = NULL;
    (*node)->up_right = NULL;
    (*node)->down_right = NULL;
    (*node)->down_left = NULL;
}

// functie pentru adaugarea valorilor rgb pentru un nod frunza
void create_leaf(quad_tree *node, pixel pix)
{
    node->pix_node.R = pix.R;
    node->pix_node.G = pix.G;
    node->pix_node.B = pix.B;
}

// functie pentru eliberarea memoriei arborelui
void free_tree(quad_tree *tree)
{
    if(tree != NULL)
    {
        free_tree(tree->up_left);
        free_tree(tree->up_right);
        free_tree(tree->down_right);
        free_tree(tree->down_left);
        free(tree);
    }
}

// functie pentru a verifica daca un nod este frunza
int is_leaf(quad_tree *node)
{
    if(node->up_left == NULL && node->up_right == NULL
       && node->down_right == NULL && node->down_left == NULL)
       return 1;
    return 0;   
}

#endif