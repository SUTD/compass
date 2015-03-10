// Case Study/Copy odd

/*
 * Copies odd numbered elements of b to a contiguously.
 */
void copy_odd(int* a, int*b,  int size)
{
  int i;
  int j;
  for(i=1, j=0; i<size; i+=2, j++) 
  {
	a[j] = b[i];
  }

  for(i=1, j=0; i<size; i+=2, j++)
  {
	static_assert(a[j] == b[2*j+1]);
  }
}