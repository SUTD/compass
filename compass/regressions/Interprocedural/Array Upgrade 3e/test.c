// Interprocedural/Array Upgrade 3e

 void boo(int *a, int i)
{
	a[i] = 5;
}

int bar(int* b)
{
	return (b[0] ==5);

}

void foo(int*b, int k)
{
	b[k] = 5;
	b+=k;
	int res = bar(b);
	static_assert(res == 1);

}
