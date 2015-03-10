// buffer access/strchr 1
#include <stdlib.h>

void specify_checks()
{
	check_null();
	check_buffer();
}


int x;
char *strchr (const char *s, int c)
{
	assume(x>=0 && x<buffer_size(s));
	if(rand()) return &s[x];
	return NULL;
}


void *
buffer_get_string(int *length_ptr)
{
	void *ret;

	if ((ret = buffer_get_string_ret(length_ptr)) == NULL)
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


void foo( int* length_ptr)
{
	char* user = packet_get_string(length_ptr);
	static_assert(user != NULL);
	char* style;
	
	if ((style = strchr(user, ':')) != NULL)
		*style++ = '\0'; //ok
		
	

}
