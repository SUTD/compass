// Loop/  Loop 1d

void foo(int size, int* a)
{
	int i, j;
	for(i=0, j=0; i<size; i+=3)
	{
		j+=3;
	}
	static_assert(i==j);
	static_assert(i>=size);
	static_assert(j>=size);
	static_assert(i%3==0);
}
