// Loop/Loop 1e2

void foo(int size, int* a)
{
	int i, j;
	for(i=0, j=0; i<size; i++)
	{
		j+=3;
	}
	// should fail
	static_assert(j==i);
	static_assert(i>=size);
	static_assert(j>=size);
}
