// Intraprocedural/Array pointer in stmt guard

struct s{
	int* p;
	int a;
};


void foo(int *a, int flag)
{
	int b = 0;
	*a = 5;
	a[1] = 7;
	if(a[0]==5)
 		b = 1;
	int c = a[1];

	static_assert(b==1);
	static_assert(c==7);

	if(b==1)
		*a = 3;

	static_assert(a[0]==3);

	int e;
	if(flag)
		e = a[1];
	else e = 55;

	if(flag)
		static_assert(e==7);
	else 
		static_assert(e==55);

}