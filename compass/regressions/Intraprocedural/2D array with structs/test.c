// Intraprocedural/2D array with structs

struct bar{
	int a;
	int b;
};

void foo()
{
	struct bar bb[2][2];
	bb[1][2].a=44;
	 bb[1][2].b=55;
	int t1 = bb[1][2].a;
	int t2 = bb[1][2].b;
	static_assert(t1==44);
	static_assert(t2==55);
}
