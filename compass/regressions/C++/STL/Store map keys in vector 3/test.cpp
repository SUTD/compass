
using namespace spec;

#define NUM_EXAMS 2

#define JOE 111
#define MARY 222

void bar()
{


  map<string, vector<int> > m;
 // vector<map<int, int> > v;
  vector<string> students;

  m["joe"].push_back(66);
  m["joe"].push_back(77);

  m["mary"].push_back(88);
  m["mary"].push_back(99);
  
 map<string, vector<int> >::iterator it = m.begin();
 for(; it!= m.end(); it++)
  {	
    string student = it->first;
    students.push_back(student);

  }

  string s = students[0];
  static_assert(s == "joe"); //should fail
}