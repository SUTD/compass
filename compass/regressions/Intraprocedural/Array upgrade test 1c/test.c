// Intraprocedural/Array upgrade test 1c

#include <stdlib.h>

void zoo(int** a)
{
	int x, y;
	x=0;
	y=0;
	if(**a == 0) {
		x =1;
	}
	*a++;
	*a--;
	
	if(**a == 1) {
		y=2;
	}
		

	if(x==1) {
		static_assert(y==0);
		static_assert(**a == 0);	
	}


	
}