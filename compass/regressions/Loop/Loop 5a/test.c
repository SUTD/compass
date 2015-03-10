// Loop/Loop 5a

void foo()
{
	int i;
	int a[10];
	a[5]=7;
	for(i=0; i<10; i++)
	{
		a[i] = i+1;
	}
	// these should fail
	static_assert(a[0] ==0);
	static_assert(a[1] == 1); 
	static_assert(a[2]==2);
	

}
