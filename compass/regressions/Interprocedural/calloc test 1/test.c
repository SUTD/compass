#include <stdio.h>

void specify_checks()
{
  check_buffer();
}

void* my_calloc(int elem_size, int num_elems)
{
  return malloc(elem_size*num_elems);
}

void foo()
{
  int* buf = my_calloc(sizeof(int), 10);
  buf[9] = 2; //ok
}