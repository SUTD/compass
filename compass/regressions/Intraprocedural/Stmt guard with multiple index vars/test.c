// Intraprocedural/Stmt guard with multiple index vars
#define NULL 0

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
	int b;
	if(a->tl == NULL || a->tl->hd->next == NULL)
		b=0;
	else b=1;

	if(a->tl!= NULL && a->tl->hd->next != NULL)
		static_assert(b==1);

	else static_assert(b==0);
}