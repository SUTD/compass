// Loop/Sentinels 1c

typedef int* NULL_TERMINATED;

void foo(char* x, char* y)
{
	
	if(*x == 'a') static_assert(*x != '\0');
	

	
}
