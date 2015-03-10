// Loop/Loop 2b

void foo()
{
	int i;
	int a[5];
	for(i=0; i<5; i++)
	{
		a[i] = 0;
	}

	static_assert(a[0] ==0);
	static_assert(a[1] == 0); 
	static_assert(a[2]==0);
	static_assert(a[3]==0);
	static_assert(a[4]==0);

}
