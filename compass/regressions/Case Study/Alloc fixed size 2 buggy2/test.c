// Case Study/Alloc fixed size 2 buggy2

#include <stdlib.h>

void alloc_fixed_size_buggy(int** a, int size, int n, int k)
{
   int i;
   for( i=0; i<n; i++){
       a[i] = malloc(sizeof(int)*size);
   }
   
   for(i=0; i<n; i++)
   {
   	// should fail: could underrun if size <=0
   	buffer_safe(a[i], size-1);
   }
   
}

