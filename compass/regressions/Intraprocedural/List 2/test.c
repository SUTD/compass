// Intraprocedural/List 2

struct list;
struct list{
	int hd;
	struct list* tl;
};

// b is 1 and a is 2
void foo(struct list* l1, struct list* l2)
{
	l1->hd = 1;
	l1->tl = l2;
	l2->hd=2;
	l2->tl=0;
	int b= l1->hd;
	int a = l1->tl->hd;
	static_assert(b==1);
	static_assert(a==2);
	

}
