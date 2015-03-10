// Interprocedural/Propagate Error Report 6c

void bar(int size, int* c, int* d)
{
	//should fail
	static_assert(c[0] ==d[-size+1]);
}

void foo(int size, int* a, int* b, int y)
{
	a[0] = b[0];
	b+=size;
	bar(size, a, b);
}
