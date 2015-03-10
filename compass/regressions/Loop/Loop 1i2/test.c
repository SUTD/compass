// Loop/Loop 1i2

void foo(int size, int* a, int k)
{
	int i, j;
	int m=0;
	for(i=0, j=0; i<size; i++, j++)
	{
		m=2*i+j;
	}
	// should fail
	if(size>0) static_assert(m==3*size); 
	static_assert(i==j);
	
}

