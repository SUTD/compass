
using namespace spec;


void bar(string s)
{
  map<string, int> m;
  m["aa"] = 3;
  m["bb"] = 9;
  m["cc"] = 34;
  m[s] = 56;

  m.erase("cc");

  if(s != "aa") {
    static_assert(m["aa"] == 3);
  }
  else static_assert(m["aa"] == 56);

 if(s!= "bb")  static_assert(m["bb"] == 9);
 else static_assert(m["bb"] == 56); 

  static_assert(m.count("cc") == 0);

  if(s != "cc") {
    static_assert(m[s] == 56);
  }
  else static_assert(m[s] == 0);

}