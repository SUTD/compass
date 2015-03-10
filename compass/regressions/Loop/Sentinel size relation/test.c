// Loop/Sentinel size relation
#include <stdlib.h>

void strcpy(char* str)
{
	int i = 0;
	int s = buffer_size(str);
	
	while(*str!='\0')
	{
		i++;
		str++;
	}
	
	static_assert(i<=s);
	buffer_safe(str, 0);
	
}
