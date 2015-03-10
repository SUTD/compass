// Intraprocedural/Statement guard 9

void foo(int* a, int b, int c)
{
	if(b>0 && c>=0)
		*a = 2;
	else *a =0;

	if(b<=0 ) static_assert(*a == 0);
	else if(c<0) static_assert(*a == 0);
	else static_assert(*a == 2);
}