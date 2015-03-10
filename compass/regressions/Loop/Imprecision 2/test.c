// Loop/Loop 5

void foo(int x)
{
	int i;
	int a[10];
	a[5]=7;
	for(i=0; i<10; i++)
	{
		a[i] = x*i;
	}

	//should fail
	static_assert(a[0] ==a[1]);

}