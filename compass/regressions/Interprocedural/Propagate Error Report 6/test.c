// Interprocedural/Propagate Error Report 6

void bar(int size, int* c, int* d)
{
	static_assert(c[0] ==d[-size]);
}

void foo(int size, int* a, int* b, int y)
{
	a[0] = b[0];
	b+=size;
	bar(size, a, b);
	int x=2;
}
