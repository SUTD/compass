int x = 3;

int main(int argc, char** argv)
{
  return 0;
}


void foo()
{
  if(static_choice() != 0) {
    //should fail
    static_assert(0);
  }	

  
}