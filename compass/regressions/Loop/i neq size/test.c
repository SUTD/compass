// Loop/i neq size

void foo(char* x, int size)
{
	int i;
	for(i=0; i !=size; i++)
	{
		x[i] = 5;
	}
	
	for(i=0; i < size; i++)
	{
		static_assert(x[i] == 5);
	}
}
