// Malloc wrapper inside loop

#include <stdlib.h>


void* foo()
{
	return malloc(4);
}

int
main(int ac, char **av)
{
	char* saved_argv = malloc((ac +1) * sizeof(*saved_argv));
	if(saved_argv == NULL) return 1;
	int i;
	for (i = 0; i < ac; i++)
		saved_argv[i] =  foo(); 
		
	// should fail!
	if(ac >= 2)
		static_assert(saved_argv[0] == saved_argv[1]);
}
