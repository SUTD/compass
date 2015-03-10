// Loop/Loop break cond test 4


void foo(int x, int size, int flag)
{
	int i;
	int y;
	int t = 1;
	for(i=0;  ; )
	{
		if(y<0) {
			i = 9;
			break;
		}
		 if(i>=size)
		{
			i = 7;
			break;
		}
		
		
		
		if(x>3) {
			//should fail
			static_assert(0);
			break;
		}
		if(flag) i++;
		else i+=2;
	}
}
