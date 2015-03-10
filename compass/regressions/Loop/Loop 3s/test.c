// Loop/Loop 3s

void foo(int size, int* a, int* b)
{
	int i, j;
	for(i=0, j=0; i<size; i++, j++)
	{
		a[i] = b[j];
	}

	for(i=0; i<size; i++)
	{
		static_assert(a[i] == b[i]);
	}

}
