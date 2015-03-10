// Intraprocedural/Assume test 2

void foo(int x)
{
	assume(x>=0);
	// should fail
	static_assert(x!=1);
}