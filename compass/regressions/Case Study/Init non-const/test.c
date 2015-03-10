// Case Study/Init non-const

/*
 * Initializes all elements of a to a linear function of iteration number.
 */
void init_non_constant(int* a, int c, int size)
{
  int i;
  for(i=0; i<size; i++) 
  {
	a[i] =2*i+c;
  }

  for(i=0; i<size; i++)
  {
	static_assert(a[i] ==2*i+c);
  }
}