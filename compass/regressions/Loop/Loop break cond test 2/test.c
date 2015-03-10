// Loop/Loop break cond test 2


void foo(int x, int size)
{
	int i;
	int y;
	int t = 1;
	for(i=0;  ; i++)
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
		//should fail
		static_assert(x<=3);
		
		if(x>3) {
			break;
		}
	}
}
