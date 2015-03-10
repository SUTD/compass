// Loop/Loop 2i

void foo(int size, int* a)
{
	int i;
	int* b =a;
	for(i=0; i<size; i++, a++)
	{
		*a = 0;
	}

	for(i=0; i<size; i++, b++)
	{
		static_assert(*b==0);
	}

}