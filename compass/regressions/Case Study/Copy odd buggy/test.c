// Case Study/Copy odd buggy

/*
 * Copies odd numbered elements of b to a contiguously,
 * but asserts all of their elements are equal.
 */
void copy_odd_buggy(int* a, int*b,  int size)
{
  int i;
  int j;
  for(i=1, j=0; i<size; i+=2, j++) 
  {
	a[j] = b[i];
  }

  for(i=0; i<size; i++)
  {
	// should fail
	static_assert(a[i] == b[i]);
  }
}