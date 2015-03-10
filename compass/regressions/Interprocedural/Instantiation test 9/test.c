// Interprocedural/Instantiation test 9

void foo(int* b, int* a)
{
	*b = a[2];
}

void  bar(int* x)
{
	int y = -1;
	foo(&y, x);
	static_assert( y == x[2]);
}