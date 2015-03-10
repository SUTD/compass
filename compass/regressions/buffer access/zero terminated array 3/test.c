//buffer access/zero terminated array 3

void specify_checks()
{
	check_buffer();
}




void foo()
{
	int a[] = {1, 2, 3, 0};
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
