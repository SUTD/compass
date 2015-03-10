//buffer access/zero terminated array 2

void specify_checks()
{
	check_buffer();
}

int a[] = {1, 2, 3, 0};


void foo()
{
	int i;
	for(i=0; a[i] != 0; i++);
}

void bar()
{
	foo();
}

void baz()
{
	bar();
}
