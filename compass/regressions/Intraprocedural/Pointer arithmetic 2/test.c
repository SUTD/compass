// Intraprocedural/Pointer arithmetic 2

void foo(int *a)
{
	int* b = &a[3];
	int *c = b;
	c++;
	*c = 44;
	int t = a[4];
	static_assert(t==44);
	
}