// Loop/Sentinels 1

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
	static_assert(*x == '\0');
	
	

	
}
