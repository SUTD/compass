// Loop/Loop 1g

void foo(int size, int* a, int k)
{
	int i, j;
	int m;
	for(i=0, j=0; i<size; i++, j++)
	{
		m=i+j;
	}
	if(size>0) static_assert(m==2*size-2); 
	
}
