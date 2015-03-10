// Intraprocedural/gcc weirdness test 1


void foo(int y)
{
	if(y%8==0)
		static_assert(y != 1);
}