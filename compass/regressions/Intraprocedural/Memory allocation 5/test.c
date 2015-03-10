// Intraprocedural/Memory allocation 5

#include <stdlib.h>
struct list;
struct list {
	int hd;
	struct list* tl;
};

void foo(struct list* l, int count)
{
	if(l==NULL)
		l = malloc(sizeof(struct list));
	l->hd = count;
	static_assert(l->hd == count);
}