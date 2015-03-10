// Interprocedural/Call 35

void foo(int* a, int b)
{
	if(b == 1)
		*a = 5;
	else *a = 7;
}

void bar(int flag)
{
	int b = 1;
	int a = 0;
	foo(&a, b);
	static_assert(a==5);
}