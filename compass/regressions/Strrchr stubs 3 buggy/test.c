//Strrchr stubs 3 buggy

#include <stdlib.h>


typedef unsigned int bool;
#define true 1
#define false 0

int x;
int s_size;

char* strrchr(char* s, char c)
{
	assume_size(s, s_size);
	assume(x>=0);
	assume(x<buffer_size(s)-1);
	buffer_safe(s, x+1);
	if(unknown()) return NULL;
	return &s[x];
}

static bool
remove_parents (char *dir)
{
	char *slash;
	bool ok = true;

	strip_trailing_slashes (dir);
	while (1)
	{
		slash = strrchr (dir, '/');
		if (slash == NULL)
			break;
		//should fail
		buffer_safe(slash, 2);
		slash[1] = 0;
	}
	return ok;
}
