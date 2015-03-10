// Case Study/Alloc fixed size buggy

#include <stdlib.h>

void alloc_fixed_size_buggy(int** a, int size, int n, int k)
{
   int i;
   for( i=0; i<n; i++){
       a[i] = malloc(sizeof(int)*size);
   }
   if(n>=0 && k>=0 && k<n && size>=1) {
   	// should fail
       buffer_safe(a[k], size);
   }     
}

