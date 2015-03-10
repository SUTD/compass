// Buffer access/nested array 17

#include <stdlib.h>

void specify_checks()
{
   check_buffer();
}


struct options {
 // int x;
  char* users[10];
  int num_users;
};

void foo()
{
  struct options o;
    int i=0;
    int num_users = 11;
   for(i =0; i<num_users; i++) {
    char* x= o.users[i];  //error
  }

}

