// Intraprocedural/Array of structs 3

struct s {
	int x;
	int y[4];
	int z[6];
};

// t should be 2
void foo(int x)
{
	struct s my_structs[10];
	my_structs[0].y[2] = 3;
	my_structs[2].z[5] = 66;
	int t = my_structs[0].y[2];
	int y = my_structs[x].z[5];
	static_assert(t == 3);
	if(x == 2)
		static_assert(y == 66);
}
