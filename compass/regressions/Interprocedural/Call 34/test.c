// Interprocedural/Call 34

int foo(int a)
{
	if(a == 2)
		return 2;
	else return 0;
}

void bar()
{
	int x = 2;
	int c= foo(x);
	static_assert(c == 2);
}