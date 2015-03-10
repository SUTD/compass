// Buffer access/nested array 7

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
  o.users[9] = "fff"; // ok
}
