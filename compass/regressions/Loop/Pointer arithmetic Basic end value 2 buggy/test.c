// Loop/Pointer arithmetic Basic end value 2 buggy

void foo(int*a , int size)
{
	int i;
	int* a_start = a+1;

	for(i=0; i<size; i++)
	{
		*a = 0;
		a++;
	}

	// should fail
	if(size>=1) static_assert(a - a_start == size);
	
}
