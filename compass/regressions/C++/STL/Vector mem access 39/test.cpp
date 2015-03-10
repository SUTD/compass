void specify_checks()
{
  check_memory_leak();
  check_deleted_access();
}


using namespace spec;

void foo()
{
  vector<vector<int>* > v;

  for(int i=0; i<10; i++)
  {
    vector<int>* w = new vector<int>();
    v.push_back(w);
  }

  vector<vector<int> *>::iterator it = v.begin();
  for(; it!= v.end(); it++)
  {
    delete *it;
  }


  
  // ok

  
  

  
}