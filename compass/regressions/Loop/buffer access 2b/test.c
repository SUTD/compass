// Loop/buffer access 2b

void foo(int* x)
{
	buffer_safe(x,2);
}

void bar()
{
	int x[3];
	// should fail
	foo(&x[1]);
}
