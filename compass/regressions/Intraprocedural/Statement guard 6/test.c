// Intraprocedural/Statement guard 6

void foo(int* a, int b)
{
	if(2*b+3>0)
		*a = 2;
	else *a =0;

	if(b> -2) static_assert(*a == 2);
	else static_assert(*a == 0);
}