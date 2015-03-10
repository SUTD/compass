// Case Study/Init

/*
 * Initializes all elements of a to a specified constant c.
 */
void init(int* a, int c, int size)
{
  int i;
  for(i=0; i<size; i++) 
  {
	a[i] = c;
  }

  for(i=0; i<size; i++)
  {
	static_assert(a[i] == c);
  }
}