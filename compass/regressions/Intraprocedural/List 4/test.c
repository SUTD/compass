// Intraprocedural/List 4

struct list;
struct list{
	int hd;
	struct list* tl;
};

void foo(struct list* l1, struct list* l2)
{
	l2->tl->hd = 3;
	l1->tl = l2->tl;
	int a = l1->tl->hd; 
	static_assert(a==3);
	
	
}