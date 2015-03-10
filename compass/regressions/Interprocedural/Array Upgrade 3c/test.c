// Interprocedural/Array Upgrade 3c

 void boo(int *a, int i)
{
	a[i] = 5;
}

void bar(int* b)
{
	// should fail
	static_assert(b[0] ==5);
}

void foo(int*b, int k)
{
	b[k-1] = 5;
	b+=k;
	bar(b);
}
