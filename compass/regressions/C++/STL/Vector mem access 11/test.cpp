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

void bar(vector<A*>& a_vec, int i)
{
  delete a_vec[i];
}

void foo()
{
  vector<A*> a_vec;
  A* a1 = new A(0);
  A* a2 = new A(0);
  a_vec.push_back(a1);
  a_vec.push_back(a2);
  bar(a_vec, 0);
  bar(a_vec, 1);
  a2->x = 2; // error!

}