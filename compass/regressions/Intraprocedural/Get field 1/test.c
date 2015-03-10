// Intraprocedural/Get field 1

struct s{
	int a;
	char b;
	int c;
};

void foo(struct s s1)
{
	s1.a = 9;
	s1.b = 'b';
	s1.c = 2;
	int x = s1.a;
	int y=x;
	char z = s1.b;
	x=s1.c;
	static_assert(x==2);
	static_assert(y==9);
	static_assert(z == 'b');
}