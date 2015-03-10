// Intraprocedural/Statement guard 1

void foo(int* a, int flag)
{
	int* b = a;
	if(flag)
		*a = 2;
	else *a =0;
	
	if(flag) static_assert(*b == 2);
	if(!flag) static_assert(*b == 0);
	static_assert(*b >= 0);
}