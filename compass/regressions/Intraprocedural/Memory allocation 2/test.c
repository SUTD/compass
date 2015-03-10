// Intraprocedural/Memory allocation 2
#include <stdlib.h>


struct list;
struct list{
	int hd;
	struct list* tl;
};

void foo()
{
	struct list* l = malloc(4);
	l->hd = 2;
	l->tl = malloc(4);
	l->tl->hd = 3;
	static_assert(l->hd == 2);
	static_assert(l->tl->hd == 3);

	

}