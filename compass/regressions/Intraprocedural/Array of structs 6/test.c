// Intraprocedural/Array of structs 6

struct bar {
	int a;
	int b;
};

struct s {
	int x;
	int y;
	struct bar z[4];
	
};

// t1 is 11, t2 is 22.
void foo()
{
	struct s my_structs[10];
	struct s ss;
	ss.x = 77;
	ss.y = 88;
	ss.z[1].a = 11;
	ss.z[1].b =22;
	my_structs[2] = ss;
	int t1 = my_structs[2].z[1].a;
	int t2 = my_structs[2].z[1].b;
	static_assert(t1 == 11);
	static_assert(t2 == 22);
	static_assert(my_structs[2].x == 77);
	static_assert(my_structs[2].y == 88);
}