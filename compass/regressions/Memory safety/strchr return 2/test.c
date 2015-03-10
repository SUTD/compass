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
	return 0;
}

void foo(char* user, int c) 
{
  char* y;
  if( (y = strchr(user, c)) != 0)
      *y++ = 'a'; // should fail
}

