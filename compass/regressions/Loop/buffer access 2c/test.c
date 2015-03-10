// Loop/buffer access 2c

void foo(char* x)
{
	buffer_safe(x,2);
}

void bar()
{
	char x[3];
	foo(&x[0]);
}