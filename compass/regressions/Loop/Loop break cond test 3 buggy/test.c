// Loop/Loop break cond test 3 buggy


void foo(int* a, int size)
{
	int i = 0;

	int* a_start = a;
	while(1)
	{
		
		
		*a = 0;
		a++;
		i++;
	if(i>=size) break;
		
		
	}
	//should fail
	static_assert(a== a_start+1);
	
	
}
