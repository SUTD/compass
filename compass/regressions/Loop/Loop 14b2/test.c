// Loop/Loop 14b2

#include <stdlib.h>
void foo(int size, int* a, int* b, int* c)
{
	int i;
	b = (int*)999;
	a = (int*)888;
	c = (int*)777;
	for(i=0; i<size; i++)
	{
		int* p = malloc(4);
		if(p==NULL) b=a;
		else a =c;
	}
	static_assert(b!=c);


	
}