// Case Study/Alloc fixed size final buggy

#include <stdlib.h>

void alloc_fixed_size(int** a, int size, int elem_size)
{
   int i;
   for( i=0; i<size; i++){
       a[i] = malloc(sizeof(int)*elem_size);
   }
   
}

void test_alloc()
{
	int* a[10];
	alloc_fixed_size(&a[0], 10, 5);
	
	int i, j;
	for(i=0; i<10; i++)
	{
		for(j=0; j<=5; j++) {
			//should fail
			buffer_safe(a[i], j);
		}
	}	
	
}

