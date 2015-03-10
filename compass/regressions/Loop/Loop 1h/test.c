// Loop/Loop 1h

void foo(int size, int* a, int k)
{
	int i, j;
	int m=0;
	for(i=0, j=0; i<size; i++, j++)
	{
		m=i+j+2;
	}
	if(size>=0) static_assert(m==2*size); 
	static_assert(i==j);
}

