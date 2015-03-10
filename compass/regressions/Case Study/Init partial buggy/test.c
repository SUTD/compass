// Case Study/Init partial buggy

/*
 * Initializes num_to_init number of elements of a to 0, 
 * but asserts all elements up to size are initialized to 0.
 */
void init_partial_buggy(int* a, int size, int num_to_init)
{
  assert(num_to_init <= size);
  int i;
  for(i=0; i<num_to_init; i++) 
  {
	a[i] = 0;
  }

  for(i=0; i<size; i++)
  {
	// should fail
	static_assert(a[i] == 0);
  }
}