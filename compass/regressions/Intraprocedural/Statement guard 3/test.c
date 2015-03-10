// Intraprocedural/Statement guard 3


void foo(int* a, int flag)
{
	if(flag + 1)
		*a = 2;
	else *a =0;
	
	if(flag > -1) static_assert(*a == 2);
	else if(flag == -1) static_assert(*a == 0);
	else static_assert(*a == 2);
}