// Loop/Sentinels 1d

typedef int* NULL_TERMINATED;

void foo(char* x, char* y)
{
	// should fail
	if(*x == 'a') {
		x++;	
		static_assert(*x != '\0');
	}
	

	
}
