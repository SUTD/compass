// Intraprocedural/Statement guard with index var 3

#define NULL 0

struct list;
struct list {
	int hd;
 	struct list* tl;
};

void foo(struct list* l1)
{
	int a;
	if(l1->tl->hd==1)
		a =1;
	else 
		a=0;

	if(l1->tl->hd == 0) static_assert(a==0);
	else if(l1->tl->hd >=0 && l1->tl->hd <=1) static_assert(a==1);
	else static_assert(a==0);
	
	
}