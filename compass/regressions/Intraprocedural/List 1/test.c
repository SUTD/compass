// Intraprocedural/List 1

struct list;
struct list{
	int hd;
	struct list* tl;
};

void foo(struct list* l1, struct list* l2)
{
	l1->hd = 22;
	l1->tl->hd = 33;

	l2 = l1;
	l1 = l1->tl;
	l1->tl =l2;

	static_assert(l1->hd == 33);
	static_assert(l1->tl->hd == 22);
	static_assert(l2->hd == 22);
	static_assert(l2->tl->hd == 33);
	
}