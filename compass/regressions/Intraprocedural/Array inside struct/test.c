// Intraprocedural/Array inside struct

struct s {
	int x;
	int y[4];
	int z[6];
};

void foo(struct s a)
{
	a.y[1]=2;
	a.z[2] = 50;
	int t = a.y[1];
	int y = a.z[2];
	int* z = &a.y[0];
	z++;
	*z = 4;
	static_assert(a.y[1] == 4);
	static_assert(t ==2);
	static_assert(y==50);
}
