// Intraprocedural/Statement guard with index var 1

struct list;
struct list {
	int hd;
 	struct list* tl;
};


void foo(struct list* l)
{
	int a;
	if(l->hd == 1)
		a=2;
	else if(l->hd == 2)
		a =3;
	else if(l->hd == 1 && l->hd ==2)
		a=4;
	else a =2;

	static_assert(a>=2 && a<=3);
	if(l->hd >=1 && l->hd <=2  ) 
		static_assert(a == l->hd+1);

	if(l->hd != 1 && l->hd !=2)
		static_assert(a==2);
}