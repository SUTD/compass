// Interprocedural/Call 33

int foo(int* a)
{
	if(a[1] == 2)
		return 2;
	else return 0;
}

void bar(int* y, int* z, int flag)
{
	int* x;
	if(flag) x=y;
	else x=z;
	x[1] = 2;
	int c= foo(x);
	static_assert(c == 2);
}