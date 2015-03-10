// Interprocedural/Global initialization 1
int x= 5;
int y = 5;

void foo()
{
	static_assert(x==5);
	static_assert(y==x);
}