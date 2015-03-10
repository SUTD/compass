
using namespace spec;

#define NUM_EXAMS 2

#define JOE 111
#define MARY 222

void bar()
{


  map<int, vector<int> > m;
 // vector<map<int, int> > v;
  vector<int> students;

  m[JOE].push_back(66);
  m[JOE].push_back(77);

  m[MARY].push_back(88);
  m[MARY].push_back(99);
  
 map<int, vector<int> >::iterator it = m.begin();
 for(; it!= m.end(); it++)
  {	
    int student = it->first;
    students.push_back(student);

  }

  int s = students[0];
  static_assert(s == JOE || s == MARY);
}