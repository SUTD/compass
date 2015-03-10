// Loop/ Loop 1c2

void foo(int size, int* a)
{
	int i, j;
	for(i=0, j=0; i<size; i++)
	{
		j++;
	}
	static_assert(i==j);
	static_assert(i>=size);
	static_assert(j>=size);
	// should fail
	static_assert(i==size);
	if(size >=0) static_assert(j==size);

}
