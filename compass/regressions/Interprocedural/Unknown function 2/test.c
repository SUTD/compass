// Interprocedural/Unknown function 2

void foo(int* a)
{
	int res = unknown();
	if(res == 1)
		*a = 2;
	
}

void bar()
{
	int a = 0;
	foo(&a);
	// should fail
	static_assert(a==0);
}