// Loop/Loop break cond test 7 imprecise2


void foo(int x, int size, int flag)
{
	int i;
	int j=0;
	int y;
	int t = 1;
	for(i=0;; )
	{
		if(bar()) {
			break;
		}
		
		

		i = i+flag;
	}
	//should fail
	static_assert(i>=0); 
}
