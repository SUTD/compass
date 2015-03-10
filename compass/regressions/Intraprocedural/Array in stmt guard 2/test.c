// Intraprocedural/Array in stmt guard 2

void foo()
{
	int a[10];
	if(a[1] != a[0]) {
		if(a[1] == 0 && a[0] == 0)
			static_assert(0);
	}
}