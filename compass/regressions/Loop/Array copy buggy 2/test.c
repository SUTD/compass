// Loop/Array copy buggy 2

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = b[i+1];
	}
	
	for(i=0; i<size; i++)
	{
		static_assert(a[i] == b[i]);
	}

}
