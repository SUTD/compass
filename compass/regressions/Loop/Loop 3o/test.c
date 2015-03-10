// Loop/Loop 3o

void foo(int size, int* a, int* b)
{
	int i;
	for(i=1; i<size; i+=2)
	{
		a[i] = b[i];
	}

	if(size == 4)
	{
		static_assert(a[1] == b[1]);
		static_assert(a[3] == b[3]);
	}	

}
