// Intraprocedural/Nested pointer struct assignment

struct s1{
	int a;
	char b;
};

struct s2 {
	int c;
	struct s1 s;
} ;


void foo(struct s2* p1)
{
	p1->s.a = 55;
	p1->s.b = 'f';
	struct s2* p2 = p1;
	static_assert(p2->s.a == 55);
	static_assert(p2->s.b == 'f');
}