// Buffer access/nested array 13

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
  for(i =0; i<10; i++) {
   char* x= o.users[i]; //ok
  }

}
