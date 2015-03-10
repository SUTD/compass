// Case Study/Array append final buggy

#include <stdlib.h>

int* array_append(int* a,  int size_a, int* b, int size_b)
{	
	
	int* res = malloc(sizeof(int)*(size_a + size_b));
	int i;
	for(i=0; i<size_a; i++) {
		res[i] = a[i];
	}
	for(i=0; i<size_b; i++) {
		res[i+size_a] = b[i];
	}
	
	return res;
}

void test_append(int*a, int size_a, int* b, int size_b)
{
	int* c = array_append(a, size_a, b, size_b);
	int i=0;
	for(; i<size_a+size_b; i++)
	{
		// should fail
		if(i<=size_a) static_assert(a[i] == c[i]);
		else static_assert(c[i] == b[i-size_a]);
	}
}


