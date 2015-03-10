// Memory safety/null 2
#include <stdlib.h>


void specify_checks()
{
	check_null();
}

void foo(char** x)
{

	// Potential null deref
	char a = *x[3];
}

void bar()
{
	char* x[4] = {"a", "b", "c", NULL};
	foo(x);
}
