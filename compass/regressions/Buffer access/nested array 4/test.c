// Buffer access/nested array 4

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
   char*z = y[9]; //error!
}
