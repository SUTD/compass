// Loop/Loop 1g2

void foo(int size, int* a, int k)
{
	int i, j;
	int m;
	for(i=0, j=0; i<size; i++, j++)
	{
		m=i+j;
	}
	// should fail
	if(size>0) static_assert(m==2*size); 
	
}
