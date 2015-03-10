// Loop/i neq size buggy

void foo(char* x, int size)
{
	int i;
	for(i=0; i !=size; i++)
	{
		x[i] = 5;
	}
	
	for(i=0; i <= size; i++)
	{
		//should fail
		static_assert(x[i] == 5);
	}
}
