// Case Study/Init even

/*
 * Initializes all even elements of a to 1.
 */
void init_even(int* a,  int size)
{
  int i;
  for(i=0; i<size; i+=2) 
  {
	a[i] = 1;
  }

  for(i=0; i<size; i+=2)
  {
	static_assert(a[i] == 1);
  }
}