// Intraprocedural/Pointer array 1

void foo(int *a, int* b, int flag)
{
	a =b;
	b[2] = 77;
	b[3] = 33;
	int z = a[2];
	int y = a[3];
	static_assert(z==77);
	static_assert(y==33);
}