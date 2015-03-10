// Loop/Loop 6b

void foo()
{
	int i;
	int a[2];
	int b[2];
	a[0] = 0;
	a[1] =1;
	b[0] = 99;
	b[1]=88;

	for(i=0; i<2; i++)
	{
		int t = a[i];
		a[i] = b[i];
		b[i] = t;
	}

	static_assert(a[0] == 99);
	static_assert(a[1] == 88); 
	static_assert(b[0]==0);
	static_assert(b[1] == 1);
	

}
