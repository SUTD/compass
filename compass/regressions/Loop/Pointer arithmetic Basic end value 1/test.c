// Loop/Pointer arithmetic Basic end value 1

void foo(int*a , int size)
{
	int i;
	int* a_start = a;
	for(i=0; i<size; i++)
	{
		*a = 0;
		a++;
	}
	if(size>=1) static_assert(a ==a_start + size);
	
}
