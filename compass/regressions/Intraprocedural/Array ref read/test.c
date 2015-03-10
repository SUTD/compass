// Intraprocedural/Array ref read

void foo(int x)
{
	int a[100];
	a[0] = 7;
	a[4] = 8;
	int b = a[0];
	static_assert(b == 7);
	int i = 2*x;
	int c = a[i];
	if(x == 2)
		static_assert(c==8);
}