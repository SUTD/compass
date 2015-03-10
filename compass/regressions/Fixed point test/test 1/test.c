// Fixed point test/test 1

/* Just to check fixed point computation
  * terminates.
 */

void foo(int flag, int x)
	
{
	while(1) {
	
		switch(x) {
			case 1:
			{
				if(flag == 0) {
					flag = 1;
					x=2;	
				}
				else {
					x++;
					break;
				}
			}
		}
		
	}
}
