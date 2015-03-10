// Intraprocedural/List of lists

struct list1;
struct list2;
struct list1 {
	struct list2* hd;
	struct list1* tl;
};

struct list2{
	int count;
	struct list2* next;
};

void foo(struct list1* a)
{
	
	struct list2* b;
	b=a->tl->hd;
	b->count = 22;
	b->next->count = 33;
	int c = a->tl->hd->next->count;
	static_assert(c==33);
	static_assert(a->tl->hd->count == 22);
}