// Intraprocedural/buffer_size test 1

void foo()
{
	int x[3];
	int y= buffer_size(x);
	static_assert(y==3 *sizeof(int));
}
