// Case Study/strlen buggy
#include <stdlib.h>

/*
 *  Buggy version of strlen that has an off-by-one error.
 */
unsigned int strlen(char* str)
{
	int i=0;
	while(str[i++] != '\0')
	{
		
	}
	return i;
	
}
void check_strlen(char* str)
{
	unsigned int size = strlen(str);
	int i=0;
	while(str[i] != '\0')
	{
		i++;
	}
	
	static_assert(size == i);
}
