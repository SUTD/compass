// Buffer access/nested array 6

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
   char** y = &o.users[1];
   y++;
   char*z = y[8]; //error 
}
