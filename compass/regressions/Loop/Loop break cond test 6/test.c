// Loop/Loop break cond test 6


void foo(int x, int size, int flag)
{
	int i;
	int y;
	int t = 1;
	for(i=0;  ; )
	{
		if(y<0) {
			break;
		}
		 if(i>=size)
		{
			break;
		}
		
		
		
		if(x>3) {
			break;
		}
		if(flag) i++;
		else i+=2;
	}
	//should fail
	if(flag) static_assert(i%2==0); 
}