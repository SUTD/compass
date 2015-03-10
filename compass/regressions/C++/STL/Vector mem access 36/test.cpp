void specify_checks()
{
  check_memory_leak();
  check_deleted_access();
}


using namespace spec;

void foo()
{
  vector<vector<int>* > v;

  vector<int>* w1 = new vector<int>();
  w1->push_back(1);
  w1->push_back(2);

  vector<int>* w2 = new vector<int>();
  w2->push_back(3);
  w2->push_back(4);

  v.push_back(w1);
  v.push_back(w2);

  int x= (*w1)[1]; 


  vector<vector<int>* >::iterator it = v.begin();
  for(; it!= v.end(); it++)
  {
    delete *it;
  }
  
  
  // ok

  
  

  
}