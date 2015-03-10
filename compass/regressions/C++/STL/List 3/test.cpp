
/*
 * Buggy version of Singleton example.
 * The Singleton class implements a get_shared method for points, 
 * such that the result of get_shared should be a point with the same
 * x and y coordinates if such a point exists, otherwise, it
 * is the same point. 
 * The check_correctness function in Singleton asserts the functional 
 * correctness of the get_shared method.

 * Bug: The implementation of operator==() for points is wrong!
 */

using namespace spec;

void test()
{
  list<int> l;
  l.push_back(7);
  l.push_front(3);
  l.push_front(8);
  l.push_back(0);
  
  int x = l.front();
  static_assert(x == 8);
}