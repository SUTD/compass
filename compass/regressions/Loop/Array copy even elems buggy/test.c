// Loop/Array copy even elems buggy

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i<size; i+=2)
	{
		a[i] = b[i];
	}

	// SHOULD FAIL
	for(i=1; i<size; i+=2)
	{
		static_assert(a[i] == b[i]);
	}
		

}
