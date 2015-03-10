// Loop/i neq size 2 buggy

void foo(char* x, int size)
{
	int i;
	for(i=0; i !=size+1; i++)
	{
		x[i] = 5;
	}
	
	for(i=0; i <= size+1; i++)
	{
		//should fail
		static_assert(x[i] == 5);
	}
}
