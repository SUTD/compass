// Strrchr wrong assertion 1 
 #include "stdlib.h"
 
 
char* strrchr(const char* s, char c)
{
	while(*s!='\0') {
		if(*s == c) return s;
		s++;
	}
	
	
	return NULL;
	
}


void foo(char* s, char c)
{
	char* res = strrchr(s, c);
	if(res != NULL) static_assert(s[0] == c);
}
