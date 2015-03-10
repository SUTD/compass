// Intraprocedural/buffer size 2



void bar(int k)
{
	char x[3];
	char* f = &x[k];
	int size = buffer_size(f);
	static_assert(size == 3-k);

}
