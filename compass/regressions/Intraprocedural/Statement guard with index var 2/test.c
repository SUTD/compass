// Intraprocedural/Statement guard with index var 2

#define NULL 0

struct list;
struct list {
	int hd;
 	struct list* tl;
};

void foo(struct list* l1)
{
	struct list*  l2;
	if(l1->tl!=NULL)
		l2 = l1->tl;
	else 
		l2 = NULL;

	if(l1->tl != NULL) static_assert(l2 != NULL);
	else static_assert(l2 == NULL);
}