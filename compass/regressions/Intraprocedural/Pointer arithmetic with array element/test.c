// Intraprocedural/Pointer arithmetic with array element

void foo(int* p, int x)
{
	int* next = p +p[1];
	static_assert(next == p+p[1]);
	if(p[1] == 1) static_assert(next == &p[1]);
}