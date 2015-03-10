

int main(int argc, char** argv)
{
  return 0;
}


void foo()
{
  //should fail
  static_assert(0);
  
}