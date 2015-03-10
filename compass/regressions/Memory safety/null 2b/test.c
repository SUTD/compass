// Memory safety/null 2b
#include <stdlib.h>


void specify_checks()
{
	check_null();
}

void foo(char** x)
{

	// correct
	char a = *x[2];
}

void bar()
{
	char* x[4] = {"a", "b", "c", NULL};
	foo(x);
}
