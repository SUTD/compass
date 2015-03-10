// Loop/Init non-const

void foo(int* a, int size)
{
	int i;
	
	for(i=0; i<size; i++)
	{
		a[i] = i;
	}

	for(i=0; i<size; i++)
	{
		static_assert(a[i] == i);
	}
	

}