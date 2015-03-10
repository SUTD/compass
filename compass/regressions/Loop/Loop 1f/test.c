// Loop/Loop 1f

void foo(int size, int* a, int z)
{
	int i, j;
	for(i=0, j=0; i<size; i++)
	{
		
		if(i==z) break;
		j+=3;
	}
	static_assert(j==3*i);
	
}
