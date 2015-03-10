using namespace spec;



struct Duck
{
  int weight;
  int length;
};

void init(vector<Duck> & ducks, Duck & d)
{
  ducks.push_back(d);
}

void foo()
{
  Duck d;
  d.weight = 7;
  d.length = 14;
  
  vector<Duck> ducks;
  init(ducks, d);
  d.length = 7;
  static_assert(ducks[0].length == 7); //should fail
}