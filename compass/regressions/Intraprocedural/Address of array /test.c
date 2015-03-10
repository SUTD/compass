// Intraprocedural/Address of array 

void foo(int* b, int x)
{
	if(x<3) return;
	int a[100];
	a[0] = 1;
	a[1] =2;
	a[x] =3;
	b = &a[0];
	int c = *b;
	int d = b[1];
	int e = b[x];
	static_assert(c==1);
	static_assert(d==2);
	static_assert(e==3);

	
}
