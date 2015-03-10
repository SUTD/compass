// Intraprocedural/buffer size 5



void bar(int k)
{

	int* x = malloc(10*sizeof(int));
	x[1] = 3;
	
	int* y = &x[5];
	// should fail
	static_assert(buffer_size(y)==10);

}
