// Intraprocedural/Binops

int foo(int*a)
{
	int b = *a <2; 
	if(*a == 0) 
		static_assert(b!=0);

	int c = (b && *a);
	if(*a < 2 && *a > 0)
		static_assert(c!=0);

	int d = b || *a;
	if(*a < 2)
		static_assert(d!=0);

	int x,y,z;
	y = 3*z;
	x = y+1;
	static_assert(x == 3*z+1);
	if(z>=0) static_assert(x>z);
}