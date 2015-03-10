#include <assert.h>
int unknown1();

int main() {
  int x,m,n;

  x = 0;
  m = 0;
  while( x < n ) {
    if(unknown1())
	m = x;
    x++;
  }
  if( n > 0 )
    {
      static_assert( 0<=m);
      static_assert(m<n);
    }
}
