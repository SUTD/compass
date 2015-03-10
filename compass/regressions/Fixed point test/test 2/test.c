// Fixed point test/test 2

/* Assert should fail
 */

void foo(int flag, int x)
	
{

	x = 1;
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
		
		if(x > 2) break;
	
		
	}
	
	static_assert(0);
	
}
