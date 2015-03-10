// Interprocedural/Imprecision test 1d
int bar()
{

	if(boo())
	{
		return 0;
	}
	return -1; 

}

void foo(int* x)
{
	int c = bar();
	if(c == -1)
	{
		*x=2;
	}
	*x = 1;
}


void baz()
{
	int a = 0;
	foo(&a);
	static_assert(a == 1);
}