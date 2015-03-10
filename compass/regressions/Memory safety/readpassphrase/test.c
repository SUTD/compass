// Memory safety/readpassphrase

void specify_checks()
{
   check_null();
   check_buffer();
}

#include <stdlib.h>
char *
readpassphrase()
{
   char buf[1024];
   int bufsiz = sizeof buf;

   int nr;
   char ch, *p, *end;
      p = buf;

   end = buf + bufsiz - 1;

   for (p = buf; p<end;) {
           *p++ = ch;
       
   }

      *p = '\0';
   return(nr == -1 ? NULL : buf);
}