// Loop/Loop 4g

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = b[size-1-i];
	}


	for(i=1; i<size; i++)
	{
		static_assert(a[i] == b[size-1-i]);
	}

}