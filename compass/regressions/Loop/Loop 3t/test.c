// Loop/Loop 3t

void foo(int size, int* a, int* b)
{
	int i, j;
	for(i=0, j=1; i<size; i++, j++)
	{
		a[i] = b[j];
	}

	for(i=0; i<size; i++)
	{
		static_assert(a[i] == b[i+1]);
	}

}
