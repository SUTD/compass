// Loop/Loop 3j

void foo(int* a, int* b, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		*a= *b;
	}
	if(size >=2)
	{
		static_assert(a[0] == b[0]);	
	}
	
	

}
