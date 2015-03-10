// Interprocedural/Instantiation test 6

void foo(int* a)
{
	if(a[1] == 1)   
		a[3] = 3;
	else a[3] = -1;
}

void bar(int* a)
{
	foo(a);
	if(a[1] == 1)  
		static_assert(a[3] == 3);
	else static_assert(a[3] ==-1);
}