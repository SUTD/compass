void foo()
{
  char* a = "compass";
  char* b = "compass";
  static_assert(a == b);
}
