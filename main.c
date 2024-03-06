#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// macro pentru calcularea maximului
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// folosesc typedef-uri pentru a scurta denumirile tipurilor variabilelor
typedef unsigned long long ull;
typedef unsigned char u_chr;
typedef unsigned int u_int;

#include "include/quadtree.h"
#include "include/queue.h"

// functie pentru alocarea memoriei matricei de pixeli
pixel **init_image(u_int size)
{
    u_int i;
    pixel **image = malloc(size * sizeof(pixel*));
    for(i = 0; i < size; i++)
    {
        image[i] = malloc(size * sizeof(pixel));
    }
    return image; 
}

// functie pentru extragerea datelor din fisierul ppm
pixel **get_image(u_int *width, u_int *height, FILE *f)
{
    char file_type[3];
    fscanf(f, "%s", file_type);
    fscanf(f, "%u%u", width, height);
    u_int max_color_val;
    fscanf(f, "%u", &max_color_val);
    fseek(f, 1, SEEK_CUR);
    pixel **image = init_image(*height);
    
    u_int i, j;    
    for(i = 0; i < *height; i++)
    {
      for(j = 0; j < *width; j++)
      {
        fread(&image[i][j], sizeof(pixel), 1, f);
      }
    }
    return image;  
}

// functie pentru eliberarea memoriei matricei de pixeli
void free_image(pixel **image, u_int size)
{
    u_int i;
    for(i = 0; i < size; i++)
    {
        free(image[i]);
    }
    free(image);
}

// functie pentru calcularea culorii medii a unui bloc 
pixel pix_avr(pixel **image, u_int x, u_int y, u_int size)
{
    pixel pix;
    ull red = 0, green = 0, blue = 0;
    u_int i, j; 
        for(i = x; i < x + size; i++)
        {
            for(j = y; j < y + size; j++)
            {
                red += image[i][j].R;
                green += image[i][j].G;
                blue += image[i][j].B;
            }
        }
    pix.R = red / (size * size);
    pix.G = green / (size * size);
    pix.B = blue / (size * size);
    return pix;
}

// functie pentru calcularea scorului similaritatii pentru un bloc
ull calc_mean(pixel **image, u_int x, u_int y, u_int size, pixel pix)
{
    ull mean = 0;
    u_int i, j;
    for(i = x; i < x + size; i++)
    {
        for(j = y; j < y + size; j++)
        {
            mean += (pix.R - image[i][j].R) * (pix.R - image[i][j].R)
                 + (pix.G - image[i][j].G) * (pix.G - image[i][j].G)
                 + (pix.B - image[i][j].B) * (pix.B - image[i][j].B);
        }
    }
    mean /= (3 * size * size);
    return mean;
}


/* functie pentru construirea arobrelui folosind matricea de pixeli si
   determinarea numarului nivelelor, frunzelor si dimensiunea celei mai
   mari zone nedivizate */
void construct_tree(quad_tree *tree, pixel **image, u_int x, u_int y, u_int size, u_int factor, u_int *level, u_int *leaves, u_int *max_block) 
{
    pixel pix = pix_avr(image, x, y, size);
    ull mean = calc_mean(image, x, y, size, pix);

    /* daca mean-ul este mai mic sau egal decat factorul, inseamna ca nodul
       va fi frunza si trebuie adaugate valorile red, green, blue */
    if(mean <= factor)
    {
        create_leaf(tree, pix);
        *leaves = *leaves + 1;
        /* pentru a determina numarul de nivele calculez maximul dintre
           nivelul nodului frunza curent si nivelul ultimului nod frunza
           intalnit */
        *level = MAX(*level, tree->level);
        /* pentru a determina latura celei mai mari zone ramase nedivizate
           calculez maximul dintre latura zonei care corespunde nodului
           frunza curent si zona maxima precedenta */
        *max_block = MAX(*max_block, size);
    }
    else
    /* apelez recursiv functia pentru copiii unui nod, iar in functie de pozitia
       nodului folosesc coordonate pentru zona din matrice care ii 
       corespunde */
    {
        init_node(&tree->up_left);
        tree->up_left->level = tree->level + 1;
        construct_tree(tree->up_left, image, x, y, size / 2, factor, level, leaves, max_block);

        init_node(&tree->up_right);
        tree->up_right->level = tree->level + 1;
        construct_tree(tree->up_right, image, x, y + size / 2, size / 2, factor, level, leaves, max_block);
        
        init_node(&tree->down_right);
        tree->down_right->level = tree->level + 1;
        construct_tree(tree->down_right, image, x + size / 2, y + size / 2, size / 2, factor, level, leaves, max_block);
        
        init_node(&tree->down_left);
        tree->down_left->level = tree->level + 1;
        construct_tree(tree->down_left, image, x + size / 2, y, size / 2, factor, level, leaves, max_block);     
    }
}

// functia in care parcurg pe nivel arborele si scriu datele in fisierul binar
void img_compress(quad_tree *tree, u_int size, FILE *g)
{
    queue *q = NULL;
    init_queue(&q);
    fwrite(&size, sizeof(u_int), 1, g);
    // in node_type retin 1 daca nodul este frunza sau 0 daca nu este
    u_chr node_type;
    if(is_leaf(tree))
    {
        node_type = 1;
        fwrite(&node_type, sizeof(u_chr), 1, g);
        fwrite(&tree->pix_node, sizeof(pixel), 1, g);
    }
    else
    {
        node_type = 0;
        fwrite(&node_type, sizeof(u_chr), 1, g);   
    }
    enqueue(q, tree);
    while(!is_empty(q))
    {
        tree = dequeue(q);
        if(tree->up_left != NULL)
        {
            if(is_leaf(tree->up_left))
            {
                node_type = 1;
                fwrite(&node_type, sizeof(u_chr), 1, g);
                fwrite(&tree->up_left->pix_node, sizeof(pixel), 1, g);
            }
            else
            {
                node_type = 0;
                fwrite(&node_type, sizeof(u_chr), 1, g);
            }
            enqueue(q, tree->up_left);
        }
        if(tree->up_right != NULL)
        {
            if(is_leaf(tree->up_right))
            {
                node_type = 1;
                fwrite(&node_type, sizeof(u_chr), 1, g);
                fwrite(&tree->up_right->pix_node, sizeof(pixel), 1, g);
            }
            else
            {
                node_type = 0;
                fwrite(&node_type, sizeof(u_chr), 1, g);
            }
            enqueue(q, tree->up_right);
        }
        if(tree->down_right != NULL)
        {
            if(is_leaf(tree->down_right))
            {
                node_type = 1;
                fwrite(&node_type, sizeof(u_chr), 1, g);
                fwrite(&tree->down_right->pix_node, sizeof(pixel), 1, g);
            }
            else
            {
                node_type = 0;
                fwrite(&node_type, sizeof(u_chr), 1, g);
            }
            enqueue(q, tree->down_right);
        }
        if(tree->down_left != NULL)
        {
            if(is_leaf(tree->down_left))
            {
                node_type = 1;
                fwrite(&node_type, sizeof(u_chr), 1, g);
                fwrite(&tree->down_left->pix_node, sizeof(pixel), 1, g);
            }
            else
            {
                node_type = 0;
                fwrite(&node_type, sizeof(u_chr), 1, g);
            }
            enqueue(q, tree->down_left);
        }
    }
    free(q);
}

/* in functia de reconstruire a arborelui extrag datele din fisierul binar
   si parcurg arborele asemanator ca la functia de compresie, cu diferenta
   ca bag in coada doar nodurile care nu sunt frunze, nodurile frunza 
   legandu-le de nodul extras din coada */
void reconstruct_tree(quad_tree *tree, u_int *size, FILE *f)
{
    fread(size, sizeof(u_int), 1, f);
    // node_type este folosit pentru a citi din fisierul binar tipul nodului
    u_chr node_type;
    pixel pix;
    fread(&node_type, sizeof(u_chr), 1, f);
    if(node_type == 1)
    {
        fread(&pix, sizeof(pixel), 1, f);
        create_leaf(tree, pix);   
    }
    else
    {
        queue *q = NULL;
        init_queue(&q);
        enqueue(q, tree);   
        while(!is_empty(q))
        {
            tree = dequeue(q);
            
            // initializez toti cei 4 copii ai nodului extras
            init_node(&tree->up_left);
            tree->up_left->level = tree->level + 1;

            init_node(&tree->up_right);
            tree->up_right->level = tree->level + 1;
            
            init_node(&tree->down_right);
            tree->down_right->level = tree->level + 1;
            
            init_node(&tree->down_left);
            tree->down_left->level = tree->level + 1;
            
            /* variabilele de tip u_chr vor fi folosite pentru a extrage
               tipul nodului din fisierul binar pentru fiecare din cele 4
               noduri */ 
            u_chr uleft_node, uright_node, dright_node, dleft_node;
            fread(&uleft_node, sizeof(u_chr), 1, f);
            if(uleft_node == 1)
            {
                fread(&pix, sizeof(pixel), 1, f);
                create_leaf(tree->up_left, pix);
            }
            else
            {
                enqueue(q, tree->up_left);
            }

            fread(&uright_node, sizeof(u_chr), 1, f);
            if(uright_node == 1)
            {
                fread(&pix, sizeof(pixel), 1, f);
                create_leaf(tree->up_right, pix);
            }
            else
            {
                enqueue(q, tree->up_right);
            }
            
            fread(&dright_node, sizeof(u_chr), 1, f);
            if(dright_node == 1)
            {
                fread(&pix, sizeof(pixel), 1, f);
                create_leaf(tree->down_right, pix);
            }
            else
            {
                enqueue(q, tree->down_right);
            }

            fread(&dleft_node, sizeof(u_chr), 1, f);
            if(dleft_node == 1)
            {
                fread(&pix, sizeof(pixel), 1, f);
                create_leaf(tree->down_left, pix);
            }
            else
            {
                enqueue(q, tree->down_left);
            }

        }
        free(q);
    }
}

// functie pentru reconstruirea imaginii pe baza arborelui
void reconstruct_image(quad_tree *tree, pixel **image, u_int x, u_int y, u_int size)
{
    /* apelez recursiv functia pentru copiii unui nod pana ajung la un nod
       frunza */
    if(!is_leaf(tree))
    {
        reconstruct_image(tree->up_left, image, x, y, size / 2);
        reconstruct_image(tree->up_right, image, x, y + size / 2, size / 2);
        reconstruct_image(tree->down_right, image, x + size / 2, y + size / 2, size / 2);
        reconstruct_image(tree->down_left, image, x + size / 2, y, size / 2);
    }
    else
    /* cand ajung la un nod frunza introduc in zona matricei corespunzatoare
       nodului valorile red, green, blue din nod */
    {
        u_int i, j;
        for(i = x; i < x + size; i++)
        {
            for(j = y; j < y + size; j++)
            {
                image[i][j].R = tree->pix_node.R;
                image[i][j].G = tree->pix_node.G;
                image[i][j].B = tree->pix_node.B;
            }
        }
    }
}

// functie pentru introducerea datelor si a matricei de pixeli in fisierul ppm
void print_image(pixel **image, u_int size, FILE *g)
{
    char file_type[3] = "P6";
    u_int width = size, height = size;
    u_int max_color_val = 255;
    fprintf(g, "%s\n", file_type);
    fprintf(g, "%u %u\n", width, height);
    fprintf(g, "%u\n", max_color_val);
    u_int i;
    for(i = 0; i < size; i++)
    {
        fwrite(image[i], sizeof(pixel), size, g);
    }
}

int main(int argc, char **argv)
{
    if(strstr(argv[1], "-c"))
    {
        u_int factor = atoi(argv[2]);
        FILE *f = fopen(argv[3], "rb");
        u_int width, height;
        pixel **image = get_image(&width, &height, f);
        quad_tree *tree;
        init_node(&tree);
        // radacina se va afla la nivelul 1
        tree->level = 1;
        u_int size = height, x = 0, y = 0;
        u_int levels = 0, max_block = 0, leaves = 0;
        construct_tree(tree, image, x, y, size, factor, &levels, &leaves, &max_block);    
        if(strcmp(argv[1], "-c1") == 0)
        {
            FILE *g = fopen(argv[4], "wt");
            fprintf(g, "%u\n", levels);
            fprintf(g, "%u\n", leaves);
            fprintf(g, "%u\n", max_block);
            fclose(g);
        }
        else
        {
            FILE *g = fopen(argv[4], "wb");
            img_compress(tree, size, g);
            fclose(g);
        }
        free_image(image, size);
        free_tree(tree);
        fclose(f);   
    }
    else
    {
        FILE *f = fopen(argv[2], "rb");
        FILE *g = fopen(argv[3], "wb");
        quad_tree *tree;
        init_node(&tree);
        u_int size, x = 0, y = 0; 
        reconstruct_tree(tree, &size, f);
        pixel **image = init_image(size);
        reconstruct_image(tree, image, x, y, size);
        print_image(image, size, g);
        free_image(image, size);
        free_tree(tree);
        fclose(f);
        fclose(g);        
    }
    return 0;
}
