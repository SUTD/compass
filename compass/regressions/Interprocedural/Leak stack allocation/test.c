// Interprocedural/Leak stack allocation

// should report error
int* bar()
{
	int a;
	return &a;
}

int foo()
{
	int* b = bar();
	int x = *b;
}


