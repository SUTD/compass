// Loop/Closure test 2

void foo()
{
	int i=0;
	int j=0;
	int t;
	
	for(i=0; i<10; i++, j++)
	{
		t=i;
		i=j;
		j=t;
	}
	
	// should fail
	static_assert(t==i || t==j);
}
