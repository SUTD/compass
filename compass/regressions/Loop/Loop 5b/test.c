// Loop/Loop 5b

void foo()
{
	int i;
	int a[10];
	
	for(i=0; i<10; i++)
	{
		a[i] = i;
	}

	static_assert(a[0] ==0);
	static_assert(a[1] == 1); 
	static_assert(a[2]==2);
	

}