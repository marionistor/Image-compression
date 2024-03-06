
# PPM Image compression 

A program that compress and decompresses images in ppm format using quadtree data structures.

#Implementation

The file queue.h contains the structure and functions used for implementing queues, while the file quadtree.h contains the structure of a pixel and a quadtree, along with the functions used for initializing and freeing its memory. In the main.c file, one can find the functions corresponding to the requirements of the assignment.

For the first requirement, the tree was constructed using a recursive function in which parameters were provided to determine the matrix region corresponding to the node in the tree. The statistics were also determined within the same function.

For the second requirement, the tree was traversed level by level using a queue to store nodes from the tree.

For the third requirement, a traversal similar to the second one was employed, with the distinction that only non-leaf nodes were added to the queue, and leaf nodes were linked to the node extracted from the queue. The pixel matrix was reconstructed using a recursive function to traverse the tree.
