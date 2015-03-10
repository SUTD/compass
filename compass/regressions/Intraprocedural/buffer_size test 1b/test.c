// Intraprocedural/buffer_size test 1b

void foo()
{
	int x[3];
	int y= buffer_size(x);
	// should fail
	static_assert(y > 3*sizeof(int));
}