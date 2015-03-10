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
 {"isil", NULL},
  {NULL, NULL}
};


static void simplified_test()
{

    int i;
    for (i = 0; array[i].name != NULL; i++)
    {
     static_assert(array[i].method != NULL); // should fail
    }


    return;
}