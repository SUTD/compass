// Loop/Loop 14

void foo(int size, int* a)
{
	int i;
	for(i=0; i<size; i++)
	{
		*a = 5;	
	}
	//should fail
	if(size == 0) static_assert(a[-1] == 5);

	
}