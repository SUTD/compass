//Interprocedural/String test 1

void bar(char** s)
{
	*s = "sss";
}

	
void foo()
{
	char* ss;
	bar(&ss);
	static_assert(ss[1] == 's');
}

