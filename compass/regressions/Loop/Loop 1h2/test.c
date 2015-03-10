// Loop/Loop 1h2

void foo(int size, int* a, int k)
{
	int i, j;
	int m=0;
	for(i=0, j=0; i<size; i++, j++)
	{
		m=i+j+2;
	}
	// should fail
	if(size>=0) static_assert(m==size); 
	static_assert(i==j);
}

