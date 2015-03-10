// Intraprocedural/Put field 2

struct nested{
	int x;
	int y;
};

struct s{
	int count;
	char* id;
	struct nested n;
	int count2;
};
void foo()
{
	struct s s1;
	struct s s2;
	s1.n.x=1;
	s1.n.y = 2;
	s2 = s1;
	static_assert(s2.n.x == 1);
	static_assert(s2.n.y == 2);
	
}