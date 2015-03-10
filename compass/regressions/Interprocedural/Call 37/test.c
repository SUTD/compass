// Interprocedural/Call 37

int foo(int a, int* x)
{
	if(a+*x == 0) 
		return -1;
	else return 1;
}

void bar(int* c)
{
	int b = 1;
	*c = -1;
	int r = foo(b, c);
	static_assert(r == -1);
	
}