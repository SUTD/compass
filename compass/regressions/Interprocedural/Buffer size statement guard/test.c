// Interprocedural/Buffer size statement guard

#include <stdlib.h>


typedef unsigned int bool;
#define true 1
#define false 0



void foo(char* s)
{
	int y = buffer_size(s);
	if(y>2)
	{
		char t = s[2];
		buffer_safe(s, 2);	
	}
}
