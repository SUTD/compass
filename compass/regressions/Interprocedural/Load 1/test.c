//Interprocedural/Load 1

void foo(int** a)
{
	**a = 9;
	int* b = *a;
	int c= *b; 
	static_assert(c==9);
	static_assert(*b == 9);
	static_assert(**a == 9);
}