// Loop/Loop break cond alloc 2 buggy

#include "stdlib.h"

void foo(int** a, int* b, int size)
{

   int i;
   for(i=0; i<size; i++)
   {

	a[i] = malloc(4);
	if(a[i] == 0) break;
	

   }
  //should fail
   static_assert(a[i] == 0);

  
}
