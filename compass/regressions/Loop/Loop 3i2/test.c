// Loop/Loop 3i2

void foo(int* a, int* b, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = b[i];
	}
	// should fail
	if(size >=2)
	{
		static_assert(a[0] == b[1]);	
	}
	
	

}
