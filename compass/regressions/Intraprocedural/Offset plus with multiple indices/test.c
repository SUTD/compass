// Intraprocedural/Offset plus with multiple indices

#include <stdlib.h>
struct f{
	int a;
	int b;
};

struct s {
	struct f ff[4];
	int x; 
};


void bar()
{
	struct s a[4];
	a[1].ff[2].a = 3;
	static_assert(a[1].ff[2].a == 3);
	
}