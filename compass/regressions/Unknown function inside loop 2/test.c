// Unknown function inside loop 2

int bar()
{
	return unknown();
}

void foo()
{
	int i;
	int a[10];
	for(i=0; i<10; i++)
	{
		a[i] = bar();
	}
	// should fail
	static_assert(a[0] == a[1]); 
}
