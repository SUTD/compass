// Case Study/Copy partial

/*
 * Copies num_to_copy number of elements of b to a.
 */
void copy_partial(int* a, int*b,  int size, int num_to_copy)
{
  assert(num_to_copy <= size);
  int i;
  for(i=0; i<num_to_copy; i++) 
  {
	a[i] = b[i];
  }

  for(i=0; i<num_to_copy; i++)
  {
	static_assert(a[i] == b[i]);
  }

}