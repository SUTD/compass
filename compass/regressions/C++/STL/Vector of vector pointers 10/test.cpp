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

  v[1] = new vector<int>();
  v[1]->push_back(56);
    
  static_assert( (*w2)[0] == 56); // should fail 
  
  

  
}