// Intraprocedural/Strings 2

void foo()
{
	char* a = "aaa";
	char x = a[0];
	static_assert(x == 'a');
	static_assert(a[1] == 'a');
	static_assert(a[0] == a[2]);



}