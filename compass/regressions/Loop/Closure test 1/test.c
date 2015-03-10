// Loop/Closure test 1

void foo(int size, int* a, int x, int y, int z, int w)
{
	int i;
	for(i=0; i<size; i++)
	{
		*a = x+y;
		if(rand()) x=z;
		if(rand()) y=w;
	}
	
	// should fail
	static_assert(*a == x+y || *a == z+w);
}



