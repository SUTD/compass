// Buffer access/nested array 2

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
   // error
   char* x = o.users[10]; 
}
