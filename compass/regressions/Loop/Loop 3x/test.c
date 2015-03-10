// Loop/Loop 3x

void foo(int size, int* a, int* b, int m)
{
	int i;
	int* b_start = b;
	int* a_start = a;
	for(i=0; i<size; i++)
	{
		*a = *b;
		a++;
		b++;		
	}
	
	if(size >= 0 )
	{
		int * aa = a-size;
		int *bb = b-size;
		static_assert(aa==a_start);
		
		static_assert(bb ==b_start);
		
		
		for(i=0; i<size; i++)
		{
			static_assert(aa[i] == bb[i]);
		}

	}

}
