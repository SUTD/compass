// Loop/Loop break cond test 7c


void foo(int x, int size, int flag)
{
	int i;
	int y;
	int t = 1;
	for(i=0;  ; )
	{
		if(bar()) {
			break;
		}
		 if(unknown())
		{
			break;
		}
		
		
		
		if(x>3) {
			break;
		}
		i++;
	}
	// should fail
	static_assert(i>=1); 
}