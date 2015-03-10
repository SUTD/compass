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

    virtual bool is_graduate_student() = 0;

};

class Undergraduate:public Student
{
  int year;
  
  public:
  Undergraduate(int year, department_type major, University* uni):Student(major, uni)
  {
    this->year = year;
  }

   virtual bool is_graduate_student(){
    return false;
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

  virtual bool is_graduate_student()
  {
    return true;
  }
  

};

void test()
{
  University* stanford = new University("Stanford");
  University* berkeley = new University("Berkeley");
  Student* joe = new PhDStudent(MATH, "ergodic theory", stanford);
  Employee* mary = new PhDStudent(BIO, "enzymes", berkeley);
  

  static_assert(joe->get_university() == mary->get_employer()); //should fail
  
  
}