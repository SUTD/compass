// Interprocedural/Strrchr stubs 1b

#include <stdlib.h>


typedef unsigned int bool;
#define true 1
#define false 0

int x;
int s_size;

char* strrchr(char* s, char c)
{
	assume(x>=0);
	assume(x<buffer_size(s)-1);
	buffer_safe(s, x+1);
	if(unknown()) return NULL;
	return &s[x];
}


void foo(char* s)
{
	char* y = strrchr(s, 'i');
	if(y==NULL) return;
	// should fail
	buffer_safe(y, 2);
}
