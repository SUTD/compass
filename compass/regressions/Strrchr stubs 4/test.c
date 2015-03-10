//Strrchr stubs 4
#include <stdlib.h>


typedef unsigned int bool;
#define true 1
#define false 0

int x;
int s_size;

char* strrchr(char* s, char c)
{
	while(*s!='\0') {
		if(*s == c) return s;
		s++;
	}
	return NULL;
	
}

static bool
remove_parents (char *dir)
{
	char *slash;
	bool ok = true;
	
	slash = strrchr (dir, '/');
	
	while (1)
	{
		if (slash == NULL)
			break;
	}
	static_assert(slash==NULL);
	return ok;
}

