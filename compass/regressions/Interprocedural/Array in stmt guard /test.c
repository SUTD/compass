// Interprocedural/Array in stmt guard 

void foo(int*a, int* x)
{
	if(a[1] == 0) *x = 0;
	else *x = 1;
}

void boo()
{
	int a[3];
	a[1] = 0;
	int x;
	foo(a, &x);
	static_assert(x == 0);
}
