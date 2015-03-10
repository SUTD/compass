// Buffer access/nested array 11

#include <stdlib.h>

void specify_checks()
{
	check_buffer();
}


struct options {
   char* users[10];
   int num_users;
};

void foo()
{
   struct options o;

   int i=0;
   // ok
   for(i =0; i<10; i++) {
   	o.users[i] = NULL; //ok
   }

}

