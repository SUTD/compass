#include <stdio.h>
#include <stdlib.h>

int foo()
{
return 1;
};

int bar()
{
return 2;
};

struct s{
char* name;
int (*method)();
};

struct s array[] =
{
 {"tom", &foo},
 {"isil", &bar},
  {NULL, NULL}
};


static void simplified_test()
{
    int b[2];
    int i;
    for (i = 0; array[i].name != NULL; i++)
    {
     b[i] =  (*array[i].method)();;
    }

    static_assert(b[0] == 2); //wrong
    static_assert(b[1] == 1); //wrong


    return;
}