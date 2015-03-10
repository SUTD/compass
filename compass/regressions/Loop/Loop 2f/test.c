// Loop/Loop 2f

void foo(int size, int* a)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = 0;
	}
	for(i=0; i<size; i++)
	{
		static_assert(a[i] == 0);
	}

}
