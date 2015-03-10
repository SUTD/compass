void specify_checks()
{
  check_memory_leak();
}


void baz()
{
  int* a = new int[10];
  a[1] = 2; //leak
}