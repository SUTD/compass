#include <stdio.h>

void specify_checks()
{
  check_buffer();
}

void* my_calloc(int elem_size, int num_elems)
{
  return malloc(elem_size*num_elems);
}

void foo(int n)
{
  int* buf = my_calloc(sizeof(int), n);
  int i;
  for(i=0; i<=n; i++)
  {
    buf[i] = 0; //should fail
  }
}