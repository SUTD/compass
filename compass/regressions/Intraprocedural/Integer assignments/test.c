// Intraprocedural/Integer assignments
void foo()
{
	int a = 3;
	int b=a;
	int c=b;
	a=1;
	b=2;

	int temp = c;
	c=b;
	b= temp;

	static_assert(a==1);
	static_assert(c==2);
	static_assert(b==3);
}