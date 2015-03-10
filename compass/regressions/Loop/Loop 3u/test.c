// Loop/Loop 3u

void foo(int size, int* a, int* b)
{
	int i;
	int* a_start = a;
	int* b_start = b;
	for(i=0; i<size; i++)
	{
		*a = *b;
		a++;
		b++;		
	}

	for(i=0; i<size; i++)
	{
		static_assert(a_start[i] == b_start[i]);
	}

}