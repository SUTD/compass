// exit test

#include <stdlib.h>
void *
buffer_get_string(int *length_ptr)
{
	void *ret;

	if ((ret = malloc(4)) == NULL)
		exit(1);
	return (ret);
}

/*
 * Returns a string from the packet data.  The string is allocated using
 * xmalloc; it is the responsibility of the calling program to free it when
 * no longer needed.  The length_ptr argument may be NULL, or point to an
 * integer into which the length of the string is stored.
 */

void *
packet_get_string(int *length_ptr)
{
	
	void* res = buffer_get_string(length_ptr);
	static_assert(res != NULL);
	return res;
}


void foo(int* length_ptr)
{
	void* res = packet_get_string(length_ptr);
	static_assert(res != NULL);
}

