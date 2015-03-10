// Intraprocedural/Address test 2

struct s{
	void* data;
	int count;
	char id;
};

void foo(struct s x)
{
	struct s* y= &x;
	struct s** z = &y;
	y->count = 1;
	(*z)->id = 'k';
	(*z)->data = 0;
	static_assert(x.count == 1);
	static_assert(x.id == 'k');
	static_assert(x.data == 0);
	
	
}
