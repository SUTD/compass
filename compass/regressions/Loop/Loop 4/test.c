// Loop/Loop 4

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = b[size-1-i];
	}
	if(size>=3) {
		static_assert(a[0] ==b[size-1]);
		static_assert(a[1] == b[size-2]); 
		static_assert(a[2]==b[size-3]);
	}

}
