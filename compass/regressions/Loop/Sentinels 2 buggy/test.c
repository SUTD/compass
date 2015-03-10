// Loop/Sentinels 2 buggy

typedef int* NULL_TERMINATED;

void foo(char* x, char* y)
{
	int i;
	//x[0] = 'f';
	char* x_start = x;
	while(*x!='\0')
	{
		*x = 'i';
		x++;
		
	}

	// should fail
	x= x_start;
	while(*x!='\0')
	{
		static_assert(*x == 'a');
		x++;
		
	}

	
}
