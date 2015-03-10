// Loop/Loop 2h

void foo(int size, int* a)
{
	int i;
	int* b =a;
	for(i=0; i<size; i++, a++)
	{
		*a = 0;
	}
	// SHOULD FAIL
	for(i=0; i<=size; i++)
	{
		static_assert(b[i] ==0);
	}


}