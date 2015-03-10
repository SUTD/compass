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

enum university_id
{
  STANFORD,
  BERKELEY,
  HARVARD,
  CMU,
  MIT,
  PRINCETON,
  CORNELL

};

char* university_names[] =
{
  "stanford",
  "berkeley",
  "harvard",
  "cmu",
  "mit",
  "princeton",
  "cornell"
};




class Employer
{
  protected:
  char* name;
  public:
    Employer() {};
    Employer(char*name)
    {
      this->name = name;
    }

    char* get_name()
    {
      return name;
    }
  
};

class University: public Employer
{
  university_id id;
  public:
    University(university_id id):Employer()
    {	
      static_assert(id>=STANFORD && id <= CORNELL);
      char* name = university_names[id];
      this->id = id;
      this->name = name;
      
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


class StudentGroup
{
private:
  char* name;
  Student* students[3];
  int num_members;

public:
  StudentGroup(char* name)
  {
    this->name = name;
    num_members = 0;
  }

  bool add_member(Student* s)
  {
    if(num_members >= 3) return false;
    students[num_members++] = s;
    return true;
  }
  
  
};



void test()
{
  University* stanford = new University(STANFORD);
  University* mit = new University(MIT);
  University* berkeley = new University(BERKELEY);

  Student* joe = new PhDStudent(MATH, "ergodic theory", stanford);
  Student* mary = new PhDStudent(BIO, "enzymes", mit);
  Student* dave = new PhDStudent(CS, "ai", stanford);
  Student* rachel = new PhDStudent(CHEM, "polymers", berkeley);
 

  StudentGroup* sg = new StudentGroup("outdoors");
  bool success = sg->add_member(joe);
  static_assert(success);
  success = sg->add_member(mary);
  static_assert(success);
  success = sg->add_member(dave);
  static_assert(success); 

  success = sg->add_member(rachel);
  static_assert(success); //should fail
  
  
  

  
  
}