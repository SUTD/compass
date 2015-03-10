// Intraprocedural/buffer size 1



void bar()
{
	char x[3];
	char* f = &x[1];
	int size = buffer_size(f);
	static_assert(size == 2);

}
