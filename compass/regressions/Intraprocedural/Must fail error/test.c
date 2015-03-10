// Intraprocedural/Must fail error

void foo(int size, int* a)
{
	int i;
	*a = 1;
	static_assert(*a==2);

}
