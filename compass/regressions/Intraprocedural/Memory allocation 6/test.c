// Intraprocedural/Memory allocation 6

#include <stdlib.h>
struct list;
struct list {
	int* hd;
	struct list* tl;
};

void foo(struct list* l, int count)
{
	if(l->tl==NULL)
		l->tl = malloc(sizeof(struct list));
	l->tl->hd = malloc(sizeof(int));
	*l->tl->hd = count;
	static_assert(*l->tl->hd == count);
}