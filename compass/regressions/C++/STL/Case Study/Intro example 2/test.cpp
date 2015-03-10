
using namespace spec;

#define NUM_EXAMS 2

#define JOE "joe"
#define MARY "mary"


/*
 * In this example, student_scores is a mapping
 * from each student to a vector storing this
 * student's score on the i'th exam.
 *
 * Using the map student_scores, we build
 * a different mapping exam_scores, which
 * maps the i'th exam to a map from students
 * to their grades on this exam. Since
 * exam numbers are consecutive, exam_scores
 * is implemented as a vector.
 *
 * Finally, we assert that the exam_scores
 * contains the correct information.
 */

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

  static_assert(joe_1 == 66);
  static_assert(joe_2 == 77);
  static_assert(mary_1 == 88);
  static_assert(mary_2 == 99);
 






}