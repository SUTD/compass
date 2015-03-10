#include <assert.h>
int unknown1();
int unknown2();
int unknown3();
int unknown4();

void main(int n)
{
    if(n<=0) return;
    int i=0; int j =0;
    while(i<n) {

      if(i%2 == 1) j++;
      i++;
      
    }
    
    if(n%2==0) static_assert(i == 2*j);

}
