// Loop/Loop 6d

void swap(int* a, int* b)
{
	int t= *a;
	*a = *b;
	*b = t;
}

void foo(int size, int* a, int* b)
{
	int i;
	a[0] = 0;
	a[1] =1;
	b[0] = 99;
	b[1]=88;
	for(i=0; i<size; i++)
	{
		swap(&a[i], &b[i]);
	}
	if(size>=2) {
		static_assert(a[0] == 99);
		static_assert(a[1] == 88); 
		static_assert(b[0]==0);
		static_assert(b[1] == 1);
	}

}