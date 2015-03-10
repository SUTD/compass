// Loop/Loop 3v

void foo(int size, int* a, int* b)
{
	int i;

	for(i=0; i<size; i++)
	{
		*a = *b;
		a++;
		b++;		
	}

	// SHOULD FAIL
	for(i=0; i<size; i++)
	{
		static_assert(a[i] == b[i]);
	}

}