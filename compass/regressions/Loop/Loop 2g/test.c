// Loop/Loop 2g

void foo(int size, int* a)
{
	int i;
	int* b =a;
	for(i=0; i<size; i++, a++)
	{
		*a = 0;
	}

	for(i=0; i<size; i++)
	{
		static_assert(b[i] ==0);
	}
}
