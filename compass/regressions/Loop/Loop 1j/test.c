// Loop/Loop 1j

void foo(int size, int* a, int k)
{
	int i, j;
	int m=0;
	for(i=0, j=0; i<size; i++, j++)
	{
		m=2*i+2*j;
	}
	if(size>0) static_assert(m==4*size-4); 
	static_assert(i==j);
	
}

