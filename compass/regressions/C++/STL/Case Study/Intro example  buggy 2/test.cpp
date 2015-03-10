
using namespace spec;

#define NUM_EXAMS 2

#define JOE "joe"
#define MARY "mary"

void bar()
{


  map<string, vector<int> > student_scores;
  vector<map<string, int> > exam_scores;

  student_scores[JOE].push_back(66);
  student_scores[JOE].push_back(77);

  student_scores[MARY].push_back(88);
  student_scores[MARY].push_back(99);

  for(int i=0; i<2; i++)
  {
    map<string, int> t;
    exam_scores.push_back(t);
  }

 map<string, vector<int> >::iterator it = student_scores.begin();
 for(; it!= student_scores.end(); it++)
  {
    string student = it->first;
    vector<int>& scores_vec = it->second;

    for(int i=0; i<2; i++)
    {
      map<string, int>& t = exam_scores[i];
      t[student] = scores_vec[i];
    }
  }

  int joe_1 = exam_scores[0][JOE];
  int joe_2 = exam_scores[1][JOE];
  int mary_1 = exam_scores[0][MARY];
  int mary_2 = exam_scores[1][MARY];

  static_assert(joe_1 == 77); // wrong
  static_assert(joe_2 == 77);
  static_assert(mary_1 == 88);
  static_assert(mary_2 == 99);
 






}