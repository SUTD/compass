// Case Study/Reverse buggy

/*
 * Reverses array a, but has an of--by-one error
 * in checking it is reversed.
 */
void reverse_buggy(int* a, int size, int k)
{	
	int i;
	int* a_copy = malloc(sizeof(int)*size);
	for(i=0; i<size; i++)
	{
		a_copy[i] = a[i];
	}
	
	for(i=0; i<size; i++)
	{
		a[i] = a_copy[size-1-i];
	}
	
	if(k>=0 && k<=size)
	{
		// should fail; the assertion does not hold if k==size.
		static_assert(a[k] == a_copy[size-1-k]);
	}

	free(a_copy);	
}