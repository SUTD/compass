// Loop/Loop break cond alloc 2

#include "stdlib.h"

void foo(int** a, int* b, int size)
{

   int i;
   for(i=0; i<size; i++)
   {

	a[i] = malloc(4);
	if(a[i] == 0) break;
	

   }
   if( i < size)
   	static_assert(a[i] == 0);

  
}
