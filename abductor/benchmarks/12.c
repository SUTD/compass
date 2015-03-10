#include <assert.h>
int unknown1();
int unknown2();
int unknown3();
int unknown4();

void main(int n) {
  int x,m;

  x = 0;
  m = 0;
  while( x < n ) {
    if(unknown())
	m = x;
	x++;
  }
  if( n > 0 )
    {
      static_assert( 0<=m && m<n);
    }
}

