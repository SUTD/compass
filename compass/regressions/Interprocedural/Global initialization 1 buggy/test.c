// Interprocedural/Global initialization 1 buggy
int x= 5;
int y = 5;

void foo()
{
	static_assert(x==5);
	//should fail
	static_assert(y==x+1);
}