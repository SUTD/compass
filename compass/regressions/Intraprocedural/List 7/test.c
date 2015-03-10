// Intraprocedural/List 7
#include <stdlib.h>

struct list;
struct list{
	int hd;
	struct list* tl;
};

// b is 1 and a is 2
struct list* foo(struct list* l1)
{
	if(l1==NULL) return;
	l1->hd=1;
	struct list* c = malloc(sizeof(struct list));
	c->hd = l1->hd;
	static_assert(c->hd==1);
	return c;
	
}