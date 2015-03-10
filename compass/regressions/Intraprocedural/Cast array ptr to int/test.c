// Intraprocedural/Cast array ptr to int

void foo(int *a) {

	a[2] = 5;
	long int b = (long int)&a[1];

	b = b+4;
	int* c = (int*)b;
	int x = *c;
	static_assert(x==5);
	

}