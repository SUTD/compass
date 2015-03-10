// Loop/Loop 3d

void boo(int* a, int* b)
{
	a[0]=b[0];
	a[1]=b[1];
}

void foo()
{
	int i;
	int a[3];
	int b[3];
	b[0]=1;
	boo(a, b);
	static_assert(a[0]==1);
	for(i=0; i<3; i++)
	{
		a[0] = b[0];
		a[1] = b[1];
	}
	static_assert(a[0]==1);
	

}
