// Loop/Loop 3w

void foo(int size, int* a, int* b)
{
	int i;

	for(i=0; i<size; i++)
	{
		*a = *b;
		a++;
		b++;		
	}

	for(i=0; i<size; i++)
	{
		static_assert(a[i-size] == b[i-size]);
	}

}
