// Intraprocedural/Pointer array 3

void foo(int **a)
{


	int* c;
	**a=7;
	int b= **a;
	a[1] = c;
	static_assert(b==7);
	static_assert(a[1] == c);
}