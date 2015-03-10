// Intraprocedural/Integer arithmetic

void foo(int a)
{
	int b=a+3;
	int c = b+2;
	int d=c+2*a;
	static_assert(d==3*a+5);

}