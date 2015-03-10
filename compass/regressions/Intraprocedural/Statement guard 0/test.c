// Intraprocedural/Statement guard 0

void foo(int flag)
{
	int a;
	if(flag)
		a = 2;
	if(flag) static_assert(a==2);
}