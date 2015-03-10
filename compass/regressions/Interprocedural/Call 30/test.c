// Interprocedural/Call 30

void foo(int** a)
{
	*a[4] = 66;;
}

void bar(int* x, int* y)
{
	*y = -1;
	int* array[10];
	array[4] = x;
	array[5] = y;
	foo(array);
	static_assert(*x == 66);
	static_assert(*y==-1);
	
	

}