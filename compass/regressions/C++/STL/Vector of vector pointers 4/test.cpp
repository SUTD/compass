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
  
  static_assert( (*w1)[0] == 1); //error

  
}