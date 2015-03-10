// Loop/buffer access 2a

void foo(int* x)
{
	buffer_safe(x,2);
}

void bar()
{
	int x[3];
	foo(&x[0]);
}