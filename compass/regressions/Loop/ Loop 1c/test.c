// Loop/ Loop 1c

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
	if(size >=0) static_assert(i==size);
	if(size >=0) static_assert(j==size);

}
