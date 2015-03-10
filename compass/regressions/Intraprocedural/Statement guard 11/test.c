// Intraprocedural/Statement guard 11

void foo(int* a, int* b)
{
	if(*b == 0)
		*a=0;
	else *a = 1;

	if(*b>0) static_assert(*a==1);
	else if(*b<0) static_assert(*a == 1);
	else static_assert(*a == 0);
	if(*b > 0 && *b < 0)
		static_assert(0==1);
}