// Interprocedural/Instantiation test

void foo(int* a)
{
	if(a[1] == 2)
		a[2] =2;
}

void  bar(int* x)
{
	x[0] = 5;
	foo(x+1);
	if(x[2] == 2) static_assert(x[3] == 2);
}
