// Interprocedural/Instantiation test 5

int foo(int* a)
{
	if(a[1] == 1 && a[2] ==2)
		return 1;
	return -1;
}

void bar(int* a)
{
	int res = foo(a+1);
	if(a[2] == 1 && a[3] ==2)
		static_assert(res == 1);
	else static_assert(res==-1);
}
