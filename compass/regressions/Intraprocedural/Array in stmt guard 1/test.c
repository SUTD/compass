// Intraprocedural/Array in stmt guard 1

void foo()
{
	int a[10];
	if(a[1] != a[0])
		static_assert(a[1]!=a[0]);
}
