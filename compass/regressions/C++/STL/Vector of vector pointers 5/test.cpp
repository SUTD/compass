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

  vector<int>* w3 = v[0];
  (*w3)[0] = 44;
  (*w3)[1] = 55;
  
  static_assert( (*w1)[0] == 44); 
 static_assert( (*w1)[1] == 55); 
  static_assert( (*w2)[0] == 3); 
 static_assert( (*w2)[1] == 4); 

  
}