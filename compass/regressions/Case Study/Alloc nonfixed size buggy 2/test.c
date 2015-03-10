// Case Study/Alloc nonfixed size buggy 2

#include <stdlib.h>

void alloc_nonfixed_size(int** a, int size, int* size_array)
{
   int i;
   for( i=0; i<size; i++){
       a[i] = malloc(sizeof(int)*size_array[i]);
   }
   
}

void test_alloc()
{
	int* a[4];
	int b[4] = {1, 2, 3, 4};
	alloc_nonfixed_size(&a[0], 4, b);
	
	int i, j;
	for(i=0; i<4; i++)
	{
		for(j=0; j<=b[i]; j++) {
			// should fail
			buffer_safe(a[i], j+1);
		}
	}	
	
}


