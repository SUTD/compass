// Intraprocedural/Statement guard 8

void foo(int* a, int b, int c)
{
	if(2*b+3>c+5)
		*a = 2;
	else *a =0;
	
	if(2*(b-1) > c) static_assert(*a == 2);
	else static_assert(*a == 0);
}