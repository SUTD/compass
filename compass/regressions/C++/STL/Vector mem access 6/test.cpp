using namespace spec;

void specify_checks()
{
  check_deleted_access();
  check_memory_leak();
}

class A
{
  public:
  int x;
  A(int x)
  {
    this->x = x;
  }
};

void bar(vector<A*>& a_vec)
{
  delete a_vec[0];
}

void foo()
{
  vector<A*> a_vec;
  int size = a_vec.size();
  A* a1 = new A(0);
  a_vec.push_back(a1);
  bar(a_vec); // no error

}