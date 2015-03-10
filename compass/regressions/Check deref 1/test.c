// Check deref 1
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
	// should fail
	foo(y);
}
