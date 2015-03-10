#include <assert.h>
int unknown1();
int unknown2();
int unknown3();
int unknown4();

void main(int n)
{
 int k=1;
 int i=1;
 int j=0;
 while(i<n) {
  static_assert(k>=i);
  j=0;
  while(j<i) {
      k += (i-j);
      j++;
  }
  i++;
 }
 
}

