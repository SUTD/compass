void foo()
{
  char* a = "compass";
  char* b = "sail";
  static_assert(a == b);
}
