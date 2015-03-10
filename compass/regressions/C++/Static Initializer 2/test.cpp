#include <stdlib.h>

int k = 6;

int flag = 0;

int* a;

int* foo()
{
   static int*a = (int*) malloc(4);

 
   return a;
}
void test()
{
   int* x = foo();
   int* y= foo();
   static_assert(x != y); //should fail

}