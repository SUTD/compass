// Loop/Loop 1e

void foo(int size, int* a)
{
	int i, j;
	for(i=0, j=0; i<size; i++)
	{
		j+=3;
	}
	static_assert(j==3*i);
	static_assert(i>=size);
	static_assert(j>=size);
}
