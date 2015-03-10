

int flag = 0;

int* foo()
{
  static int*a = 0;
  if(flag == 0) {
    a= malloc(4);
    flag =1;
  }
  
  return a;
}
void test()
{
  int* x = foo();
  int* y= foo();
  static_assert(x == y); 

}