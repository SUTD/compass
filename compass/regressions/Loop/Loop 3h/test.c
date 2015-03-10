// Loop/Loop 3h

void foo(int* a, int* b, int size)
{
	int i;
	b[0] = 0;
	b[1] = 1;
	b[2] = 2;
	for(i=0; i<size; i++)
	{
		a[i] = b[i];
	}
	if(size >=3)
	{
		static_assert(a[0] == 0);
		static_assert(a[1] == 1);
		static_assert(a[2] == 2);
	}
}
