// Loop/Writing to concrete source buggy


void bar(int* a, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[3] = i;
	}
	//should fail
	static_assert(a[3] == size-1);
}
