// Intraprocedural/Statement guard 7

void foo(int* a, int b, int c)
{
	if(2*b+3>c)
		*a = 2;
	else *a =0;

	if(2*b+3>c) static_assert(*a == 2);
	else static_assert(* a == 0);

	if(b == 2 && c== 0) static_assert(*a == 2);
	if(b==-1 && c ==1) static_assert(*a == 0);
}