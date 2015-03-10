#include <assert.h>
int unknown1();
int unknown2();
int unknown3();
int unknown4();

int i=0; int j=1;
void main(int n)
{
 int i=0;  
 int j= 1;
 while(i<n){
   if(j%2 == 1) i++;
   if(i%2 == 1) j++;
 }
 

 static_assert(j>=i);
}

