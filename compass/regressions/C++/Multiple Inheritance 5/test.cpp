#include <stdlib.h>

enum department_type
{
  CS,
  MATH,
  EE,
  PHYSICS,
  BIO,
  CHEM,
  ENGLISH,
  ECON
};


class Employer
{
  char* name;
  public:
    Employer(char*name)
    {	
      this->name = name;
    }
  
};

class University: public Employer
{
  public:
    University(char*name):Employer(name)
    {	
    };
  
};


class Employee
{
protected:
   Employer* emp;

  public:
  Employee(Employer* emp)
  {
    this->emp = emp;
  }

  Employer* get_employer()
  {
    return emp;
  }
  
};

class Student
{
  protected:
    department_type department;
    University* uni;
    
  public:

    Student(department_type department, University* uni)
    {
      this->department = department;
      this->uni = uni;
    }

    department_type get_department()
    {	
      return department;
    }

    University* get_university()
    {
      return uni;
    }

};


class PhDStudent:public Employee, public Student
{
  protected:
    char* thesis_topic;
  public:
    
  PhDStudent(department_type department, char* thesis_topic, University* uni)
   : Student(department, uni), Employee(uni)
  {
    this->thesis_topic = thesis_topic;
  }

  char* get_thesis_topic()
   {
    return thesis_topic;
  }
  

};

void test()
{
  University* stanford = new University("Stanford");
  PhDStudent* joe = new PhDStudent(MATH, "ergodic theory", stanford);

  static_assert(joe->get_department() == MATH); // ok

  Employer* joes_emp = joe->get_employer();
  University* joes_uni = joe->get_university();
  

  static_assert(joes_emp != joes_uni); // should fail
  
  
}