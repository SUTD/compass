// Buffer access/nested array 1

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
   char* x = o.users[9]; // ok
}
