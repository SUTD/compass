// Intraprocedural/buffer size 4



void bar(int k)
{

	int* x = malloc(10*sizeof(int));
	x[1] = 3;
	
	int* y = &x[5];
	static_assert(buffer_size(y)==5*sizeof(int));

}

