// Loop/Loop 3n

void foo(int size, int* a, int* b)
{
	int i;
	for(i=1; i<size; i+=2)
	{
		a[i] = b[i];
	}
	// Both assertions should fail
	if(size == 4)
	{
		static_assert(a[0] == b[0]);
		static_assert(a[2] == b[2]);
	}	

}
