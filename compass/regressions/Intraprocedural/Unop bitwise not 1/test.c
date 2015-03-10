// Intraprocedural/Unop bitwise not 1

void foo(int a)
{

	int b= ~a;
	int c= ~b;
	static_assert(c==a);
}