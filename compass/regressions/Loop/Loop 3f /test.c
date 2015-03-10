// Loop/Loop 3f 

void foo()
{
	int i;
	int a[3];
	int b[3];
	b[0] = 0;
	for(i=0; i<3; i++)
	{
		a[i] = b[i];
	}
	static_assert(a[0]==0);
	static_assert(a[1]==b[1]);
	static_assert(a[2]==b[2]);
	

}
