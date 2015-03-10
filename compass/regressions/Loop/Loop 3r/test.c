// Loop/Loop 3r

void foo(int size, int* a, int* b)
{
	int i, j;
	for(i=0, j=0; i<size; i++, j++)
	{
		a[i] = b[j];
	}
	if(size>=3) {
		static_assert(a[0] ==b[0]);
		static_assert(a[1] == b[1]); 
		static_assert(a[2]==b[2]);
	}

}
