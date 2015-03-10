// Intraprocedural/Statement guard 2


void foo(int* a, int* b)
{
	int t = *b>0;
	if(t+3!=0)
		*a=0;
	else *a = 1;
	
	static_assert(*a == 0);
	static_assert(*a!=1);
	
}