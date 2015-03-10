#include <assert.h>
int unknown1();
int unknown2();
int unknown3();
int unknown4();

void main() {
  int i;
  int j;
  int x = i;
  int y = j;
  int l = 0;
  while(x!=l) {
	x--;
	y--;
  }
  if(i==j) static_assert(y==l);
}

