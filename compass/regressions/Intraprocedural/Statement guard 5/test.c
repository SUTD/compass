// Intraprocedural/Statement guard 5

void foo(int* a, int* b)
{
	if(b<0)
		*a = 2;
	else *a =0;
	static_assert(*a == 0);
}