// Interprocedural/Instantiation test 3

void foo(int* a)
{
	a[2] =2;
}

void  bar(int* x)
{
	foo(x-1);
	static_assert(x[1] == 2);
}
