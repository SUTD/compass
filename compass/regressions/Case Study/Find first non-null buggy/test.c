// Case Study/Find first non-null buggy

#include <stdlib.h>

/*
 * Looks for a non-null element in array and sets
 * pos to the index of this element. 
 */
void find_first_nonnull(int* a, int size)
{
	int pos = -1;
	int i;
	for(i=0; i<size; i++)
	{
		if(a[i] != NULL) {
			pos = i;
			break;
		}
	}
	// should fail
	static_assert(a[i] !=NULL);
}
