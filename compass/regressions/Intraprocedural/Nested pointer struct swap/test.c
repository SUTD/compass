// Intraprocedural/Nested pointer struct swap

struct s1{
	int a;
	char b;
};

struct s2 {
	int c;
	struct s1 s;
} ;


void foo(struct s2* p1, struct s2* p2)
{
	p1->s.b = 'a';
	p2->s.b = 'b';

	struct s2* t = p2;
	p2 = p1;
	p1= t;

	static_assert(p1->s.b == 'b');
	static_assert(p2->s.b == 'a');

}