// Interprocedural/Array Upgrade 3a

 void boo(int *a, int i)
{
	a[i] = 5;
}



void foo(int*b, int k)
{
	b[k] = 5;
	b+=k;
	static_assert(b[0] == 5);
}
