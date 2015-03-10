// Intraprocedural/Array of structs 2

struct s {
	int x;
	int y;
	int z[3];
};

void foo(int flag)
{
	struct s a[10];
	a[0].z[1]=1;
	int* t = &a[0].z[1];
	t[1] = 4;
	static_assert(a[0].z[2] == 4);

	
}
