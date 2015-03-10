// Intraprocedural/Illegal cast

struct s1{
	int x;
 	int y;
};

struct s2 {
	int* p;
};

void foo(struct s1* a)
{
	struct s1* c=a;
	struct s2* b = (struct s2*)a;
	
}