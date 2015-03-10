// Loop/Init non-const buggy 2


void foo(int* a, int size)
{
	int i;
	
	for(i=0; i<size; i++)
	{
		a[i] = i;
	}
	// SHOULD FAIL
	for(i=0; i<=size; i++)
	{
		static_assert(a[i] == i);
	}
	

}