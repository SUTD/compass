// Intraprocedural/List 6

struct list;
struct list{
	int hd;
	struct list* tl;
};

void foo(struct list* l2)
{
	l2->tl->hd = 5;
	l2->tl->tl->hd = 6;
	struct list ll;
	ll = *l2->tl;
	static_assert(ll.hd == 5);
	static_assert(ll.tl->hd == 6);	
}