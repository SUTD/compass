// Loop/Loop 5h

void foo(int* a, int size)
{
	int i;
	int* a_start = a;
	for(i=0; i<size; i++)
	{
		*a = i;
		a++;
	}

	for(i=0; i<size; i++)
	{
		static_assert(a_start[i] == i);
	}
}
