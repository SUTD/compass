// Case Study/Copy

/*
 * Copies elements of b to a.
 */
void copy(int* a, int*b,  int size)
{
  int i;
  for(i=0; i<size; i++) 
  {
	a[i] = b[i];
  }

  for(i=0; i<size; i++)
  {
	static_assert(a[i] == b[i]);
  }
}
