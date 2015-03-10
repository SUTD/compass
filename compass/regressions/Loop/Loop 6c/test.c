// Loop/Loop 6c

void foo()
{
	int i;
	int a[3];
	int b[3];
	a[0] = 0;
	a[1] =1;
	a[2] = 2;
	b[0] = 99;
	b[1]=88;
	b[2] = 77;

	for(i=0; i<3; i++)
	{
		int t = a[i];
		a[i] = b[i];
		b[i] = t;
	}

	static_assert(a[0] == 99);
	static_assert(a[1] == 88); 
	static_assert(a[2] == 77); 
	static_assert(b[0]==0);
	static_assert(b[1] == 1);
	static_assert(b[2] == 2);
	

}