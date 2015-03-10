// Loop/Loop break cond alloc

#include "stdlib.h"

void foo(int** a, int* b, int size)
{

   int i;
   for(i=0; i<5; i++)
   {

	a[i] = malloc(4);
	if(a[i] == 0) break;
	

   }
   if(i<5) static_assert(a[i] == 0);

  
}
