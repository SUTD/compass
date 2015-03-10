// Intraprocedural/Array read-write 1

void foo()
{
	int a[5];
	a[1] = 1;
	int b = a[1];
	static_assert(b==1);
}