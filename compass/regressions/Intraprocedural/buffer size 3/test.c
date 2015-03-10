// Intraprocedural/buffer size 3



void bar(int k)
{

	int a[3];
	int* b = &a[1];
	static_assert(buffer_size(b)==2*sizeof(int));

}

