// Intraprocedural/Assume test 1

void foo(int x)
{
	assume(x>=0);
	static_assert(x!=-1);
}