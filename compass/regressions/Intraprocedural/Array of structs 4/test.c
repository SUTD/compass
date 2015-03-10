// Intraprocedural/Array of structs 4

struct s {
	int x;
	int y[4];
};


void foo()
{
	struct s my_structs[10];
	my_structs[0].y[2] = 3;
	struct s ss= my_structs[0];
	int t = ss.y[2];
	static_assert(t == 3);
}
