// Intraprocedural/Unop negate 1

void foo(int a)
{
	int b= -a;
	int* c = &b;
	*c = -b;

	static_assert(*c == a);
	
}