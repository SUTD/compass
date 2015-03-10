// Loop/Loop 4b2

void foo()
{
	int i;
	int a[3];
	int b[3];
	b[0] = 0;
	b[1] = 1;
	b[2] = 2;

	for(i=0; i<3; i++)
	{
		a[i] = b[i];
	}

	// all should fail
	static_assert(a[0] ==2);
	static_assert(a[1] == 1); 
	static_assert(a[2]==0);
	

}
