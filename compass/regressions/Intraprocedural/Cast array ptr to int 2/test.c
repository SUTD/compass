// Intraprocedural/Cast array ptr to int 2

struct s
{
	int x;
	int y;
};


void foo() {

	struct s a;
	a.x = 1;
	a.y=3;
	long int b = (long int)&a.x;
	long int c = b+4;
	int* d = (int*)c;
	static_assert(*d == 3);
	

}