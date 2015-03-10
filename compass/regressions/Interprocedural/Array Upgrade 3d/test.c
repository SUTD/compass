// Interprocedural/Array Upgrade 3d

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
	b[k-1] = 5;
	b+=k;
	int res = bar(b);
	//should fail
	static_assert(res == 1);

}
