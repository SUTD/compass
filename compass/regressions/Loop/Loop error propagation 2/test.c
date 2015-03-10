// Loop/Loop error propagation 2

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = 0;
	}

	// should fail
	for(i=0; i<=size; i++)
	{
		static_assert(a[i] == 0);
	}
		

}
