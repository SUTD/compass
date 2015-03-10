#include <assert.h>
int unknown1();
int unknown2();
int unknown3();
int unknown4();

void main(int n)
{
  assume(n>=0);
  int i=0, j=0;
  while(i<n) {
    i++;
    j++;
  }	
  static_assert(j==n);
}

