// Check deref 3
#include <stdlib.h>

void specify_checks()
{
	check_null();
}

void foo(int* x)
{
	int a = *x;
	int b = *x;

}

void bar(int flag)
{
	int* y;
	if(flag) 
		y = NULL;
	else {
		y = malloc(sizeof(int));
		if(y== NULL) exit(1);
	}
	// should fail
	foo(y);
}
