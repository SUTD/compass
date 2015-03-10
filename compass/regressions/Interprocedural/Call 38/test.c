// Interprocedural/Call 38

int foo(int a, int* x)
{
	if(2*a+*x == 0) 
		return -1;
	else return 1;
}

void bar()
{
	int b = 1;
	int c = -2;
	int r = foo(b, &c);
	static_assert(r == -1);
	
}