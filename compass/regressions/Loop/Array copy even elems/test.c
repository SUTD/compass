// Loop/Array copy even elems

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i<size; i+=2)
	{
		a[i] = b[i];
	}

	
	for(i=0; i<size; i+=2)
	{
		static_assert(a[i] == b[i]);
	}
		

}
