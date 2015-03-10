// Loop/Loop 5i

void foo(int* a, int size)
{
	int i;
	int* a_start = a;
	for(i=0; i<size; i++)
	{
		*a = i;
		a++;
	}

	// SHOULD FAIL
	for(i=0; i<size; i++)
	{
		static_assert(a[i] == i);
	}	

}
