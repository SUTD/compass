#include <assert.h>

int main(int i, int j) {
  
  int x = i;
  int y = j;
  while(x!=0) {
	x--;
	y--;
  }
  if(i==j)
	static_assert(y==0);
}
