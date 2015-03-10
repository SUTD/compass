// Intraprocedural/Bitwise not test


	
void foo(int x)
{
	int y=~x;
	static_assert(y== -1-x);	
}