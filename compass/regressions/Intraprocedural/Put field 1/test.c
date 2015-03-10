// Intraprocedural/Put field 1

struct s{
	int count;
	char* id;
	int count2;
};
void foo(char* b)
{
	struct s s1;
	s1.count = 1;
	s1.id = b;
	s1.count2 = 2;

	// swap count and count2
	int t = s1.count;
	s1.count = s1.count2;
	s1.count2 = t;

	static_assert(s1.count == 2);
	static_assert(s1.count2==1);
	static_assert(s1.id == b);
	
}