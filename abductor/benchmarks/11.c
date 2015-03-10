#include <assert.h>
int unknown1();
int unknown2();
int unknown3();
int unknown4();

void main() {
  int n;
  assume(n>0);
  

  int j = n;
  int k = n;
  int l =0;
  while( j > l ) {
	j--;
	k--;
  }
  static_assert(k>=l);
}
