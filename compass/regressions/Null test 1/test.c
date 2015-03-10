// Intraprocedural/gcc weirdness test 1


void specify_checks()
{
  check_null();
}

void foo(int y)
{
  int * x = 0;
  *x = 44;
  static_assert(x != 0);
  exit(0);
}