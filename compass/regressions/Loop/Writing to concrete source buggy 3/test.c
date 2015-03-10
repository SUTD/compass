// Loop/Writing to concrete source buggy 3


void bar(int* a, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = i;
	}
	//should fail
	if(size>=1) static_assert(a[3] == size-1);
}
