// Intraprocedural/List 3

struct list;
struct list{
	int hd;
	struct list* tl;
};

// Circular list
void foo(struct list* l1, struct list* l2)
{
	l1->hd = 1;
	l1->tl = l2;
	l2->hd=2;
	l2->tl=l1;
	int a = l1->tl->tl->hd;  
	int b= l1->tl->tl->tl->hd;
	static_assert(a==1);
	static_assert(b==2);
	
}