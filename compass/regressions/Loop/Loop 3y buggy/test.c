//Loop/Loop 3y buggy

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
		// should fail
		static_assert(a[i-size+1] == b_start[i]);
	}


	
}

