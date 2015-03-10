// Loop/Init non-const buggy

void foo(int* a, int size)
{
	int i;
	
	for(i=0; i<size; i++)
	{
		a[i] = i+1;
	}

	for(i=0; i<size; i++)
	{
		static_assert(a[i] == i);
	}
	

}