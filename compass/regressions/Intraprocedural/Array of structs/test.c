// Intraprocedural/Array of structs

struct s {
	int x;
	int y;
	int* z;
};

void foo(int flag)
{
	struct s a[100];
	a[0].x =1;
	a[0].y =2;
	a[0].z[1] = 3;
	static_assert(a[0].x == 1);
	static_assert(a[0].y == 2);
	static_assert(a[0].z[1] == 3);
};
