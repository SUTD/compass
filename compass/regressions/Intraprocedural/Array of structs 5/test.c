// Intraprocedural/Array of structs 5

struct s {
	int x;
	int y;
};

void foo()
{
	struct s my_structs[10];
	struct s ss;
	ss.x = 77;
	ss.y = 88;
	my_structs[2] = ss;
	my_structs[1] = ss;
	int t1= my_structs[2].x;
	int t2= my_structs[2].y;

	static_assert(t1 == 77);
	static_assert(t2 == 88);
	static_assert(t1 == my_structs[1].x);
	static_assert(t2 == my_structs[1].y);
	
}