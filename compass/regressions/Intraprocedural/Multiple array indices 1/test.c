// Intraprocedural/Multiple array indices 1

#include <stdlib.h>

struct s {
int x[2];
};


void bar()
{


	struct s a[4];
	a[0].x[1] = 1;

	static_assert(a[0].x[1] == 1);
	a[1].x[3] = 2;
	static_assert(a[1].x[3] == 2); 

	
}