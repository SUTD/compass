// Case Study/Init even buggy

/*
 * Initializes all even elements of a to 1,
 * but asserts all odd elements are 1.
 */
void init_even_buggy(int* a,  int size)
{
  int i;
  for(i=0; i<size; i+=2) 
  {
	a[i] = 1;
  }

  for(i=1; i<size; i+=2)
  {
	// should fail
	static_assert(a[i] == 1);
  }
}