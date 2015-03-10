// Loop/Loop 4h

void foo(int size, int* a, int* b)
{
	int i;
	int* a_start = a;
	int* b_end = b+size-1;
	for(i=0; i<size; i++)
	{
		*a = *b_end;
		a++;
		b_end--;
	}

	for(i=0; i<size; i++)
	{
		static_assert(a_start[i] == b[size-1-i]);
	}


}
