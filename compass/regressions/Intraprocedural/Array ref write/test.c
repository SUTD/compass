// Intraprocedural/Array ref write

void foo(int flag)
{
	int a[100];
	a[0] =0;
	a[1]=1;
	a[2] =2;

	int i;
	if(flag) i=1;
	else i=2;

	int c = a[i];
	if(flag) static_assert(c==1);
	else static_assert(c ==2);

	
}