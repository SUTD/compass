// Loop/Loop break cond test 9b

#include "stdlib.h"

void foo(int** x, int size)
{
   int i;
   for(i=0; i<size; i++)
   {
       x[i] = malloc(sizeof(int)*size);
       if(x[i] == NULL) break;
   }
     if(i <size) static_assert(x[i] == NULL);
}