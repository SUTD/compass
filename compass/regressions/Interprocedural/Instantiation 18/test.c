// Interprocedural/Instantiation 18

int bar(int* a, int l)
{
	if(a[l] == 1)
		return 1;
	return 0;
}

int foo(int* a)
{
	int res = bar(a, 1);
	if(a[1] == 1) static_assert(res == 1);
	
}