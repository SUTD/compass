void specify_checks()
{
  check_deleted_access();
}

class A
{

public:
  A(int b){this->x = b;};
  int x;
  ~A(){};
  
};

void bar(A* a)
{
  delete a;
}

void foo(int flag)
{	
  A* a = new A(2);
  if(flag) bar(a);

  if(!flag) a->x=0; //ok


}