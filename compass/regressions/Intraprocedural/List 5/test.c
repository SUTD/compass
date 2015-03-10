// Intraprocedural/List 5

struct list;
struct list{
	int hd;
	struct list* tl;
};

void foo(struct list* l1, struct list* l2)
{
	l1 = l2;
	static_assert(l1->hd == l2->hd);
	static_assert(l1->tl == l2->tl);
	static_assert(l1->tl->hd == l2->tl->hd);
	
}