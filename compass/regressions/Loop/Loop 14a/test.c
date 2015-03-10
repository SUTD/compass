// Loop/Loop 14a

void foo(int size, int* a)
{
	int i;
	for(i=0; i<size; i++)
	{
		*a = 5;	
	}
	if(size == 1) static_assert(a[0] == 5);

	
}