// Intraprocedural/Address test 1

void foo(int a)
{
	int* b= &a;
	*b = 1;
	int** c = &b;
	**c = 2;
	static_assert(a==2);
	static_assert(*b==2);
	static_assert(**c == 2);
	
}
