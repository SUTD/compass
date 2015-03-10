#include <assert.h>
int unknown1();
int unknown2();
int unknown3();
int unknown4();

void main(int n)
{
  int i=0, j=0;
  while(i<3) {
    i++;
    j++;
  }	
  static_assert(j==3);
}
