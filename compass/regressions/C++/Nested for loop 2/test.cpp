
void foo()
{
  int a[2][3];

  for(int i=0; i<2; i++)
  {
    for(int j=0; j<2; j++) {
	a[i][j] = 0;
      }
  }

  for(int i=0; i<2; i++)
  {
    for(int j=0; j<3; j++) {
	static_assert(a[i][j] == 0); //should fail
      }
  } 

}