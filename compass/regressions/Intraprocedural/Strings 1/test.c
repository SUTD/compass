// Intraprocedural/Strings 1

void foo()
{
	char* a = "election day";
	char x = a[0];
	static_assert(x == 'e');
	static_assert(a[1] == 'l');
	static_assert(a[0] == a[2]);


}