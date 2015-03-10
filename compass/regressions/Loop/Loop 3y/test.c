//Loop/Loop 3y

void foo(int size, int* a, int* b, int t)
{
	int i;
	int* b_start = b;
	for(i=0; i<size; i++)
	{
		*a = *b;
		a++;
		b++;		
	}
	for(i =0;i<size; i++)
	{
		static_assert(a[i-size] == b_start[i]);
	}


	
}

