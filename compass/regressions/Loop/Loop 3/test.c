// Loop/Loop 3

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = b[i];
	}
	if(size>=3) {
		static_assert(a[0] ==b[0]);
		static_assert(a[1] == b[1]); 
		static_assert(a[2]==b[2]);
	}

}
