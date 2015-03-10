// Intraprocedural/Address > 0 test 1

void foo()
{
	int a;
	int *b = &a;
	static_assert(b>0);
}
