// Loop/Sentinels 4

typedef int* NULL_TERMINATED;

void foo(char* x, char* y)
{
	int i;
	char* x_start = x;
	while(*x!='\0')
	{
		*x = 'i';
		x++;
		
	}
	//correct because this loop will never execute
	while(*x!='\0')
	{
		static_assert(*x == 'g');
		x++;
		
	}

	
}
