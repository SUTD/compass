// Loop/buffer access 2d

void foo(char* x)
{
	buffer_safe(x,2);
}

void bar()
{
	char x[3];
	//should fail
	foo(&x[1]);
}
