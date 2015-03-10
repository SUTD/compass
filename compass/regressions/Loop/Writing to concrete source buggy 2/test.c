// Loop/Writing to concrete source buggy 2


void bar(int* a, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[3] = i;
	}
	//should fail
	if(size>=1) static_assert(a[3] == size);
}
