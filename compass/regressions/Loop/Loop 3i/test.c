// Loop/Loop 3i

void foo(int* a, int* b, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = b[i];
	}
	if(size >=2)
	{
		static_assert(a[0] == b[0]);	
		static_assert(a[1] == b[1]);
	}
	
	

}
