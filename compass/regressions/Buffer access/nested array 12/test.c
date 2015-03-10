// Buffer access/nested array 12

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
   for(i =0; i<11; i++) {
   	o.users[i] = NULL; //error
   }

}

