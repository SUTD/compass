// Loop/Sentinels 3

typedef int* NULL_TERMINATED;

void foo(char* x, char* y)
{
	int i;
	x[0] = 'a'; x[1] = 'b';
	char* x_start = x;
	while(*x!='\0')
	{
		*x = 'i';
		x++;
		
	}

	x= x_start;
	while(*x!='\0')
	{
		static_assert(*x == 'i');
		x++;
		
	}

	
}
