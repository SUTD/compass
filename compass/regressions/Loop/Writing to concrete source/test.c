// Loop/Writing to concrete source


void bar(int* a, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[3] = i;
	}
	if(size>=1) static_assert(a[3] == size-1);
}
