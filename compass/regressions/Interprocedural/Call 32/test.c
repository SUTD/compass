// Interprocedural/Call 32

int foo(int* a)
{
	if(a[1] == 2)
		return 2;
	else return 0;
}

void bar(int* x)
{

	int c= foo(x);
	if(x[1] == 2) static_assert(c == 2);
	if(x[1] != 2) static_assert(c == 0);
}