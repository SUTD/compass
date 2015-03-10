// Loop/Array copy buggy

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = b[i];
	}
	
	for(i=0; i<=size; i++)
	{
		static_assert(a[i] == b[i]);
	}

}
