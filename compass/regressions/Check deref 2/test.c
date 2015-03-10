// Check deref 2
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

void bar(int* y)
{
	assume(y!=NULL);
	foo(y);
}
