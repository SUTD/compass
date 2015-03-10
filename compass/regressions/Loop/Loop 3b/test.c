// Loop/Loop 3b

void foo()
{
	int i;
	int a[3];
	int b[3];
	for(i=0; i<3; i++)
	{
		a[i] = b[i];
	}

	static_assert(a[0] ==b[0]);
	static_assert(a[1] == b[1]); 
	static_assert(a[2]==b[2]);
	

}
