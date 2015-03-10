// Loop/Loop 2

void foo(int size, int* a)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = 0;
	}
	if(size>=3) {
		static_assert(a[0] ==0);
		static_assert(a[1] == 0); 
		static_assert(a[2]==0);
	}

}

