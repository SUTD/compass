// Intraprocedural/Cast 1

void foo(void* a)
{
	int* b= (int*)a;
	char* c = (char*) b;
	*b = 2;
	*c = 'a';
	static_assert(*b == 'a');
	
}