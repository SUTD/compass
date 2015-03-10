// Loop/Loop 3m

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i<size; i+=2)
	{
		a[i] = b[i];
	}

	if(size == 4)
	{
		static_assert(a[0] == b[0]);
		static_assert(a[2] == b[2]);
	}	

}
