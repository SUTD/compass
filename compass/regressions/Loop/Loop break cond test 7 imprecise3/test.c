// Loop/Loop break cond test 7 imprecise3


void foo(int x, int size, int flag)
{
	int i;
	int j=0;
	int y;
	int t = 1;
	for(i=0;j<size; j++)
	{
		
		i = i+flag;
	
		
		
	}
	//should fail
	static_assert(0);

}
