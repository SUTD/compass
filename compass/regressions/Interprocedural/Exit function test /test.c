// Interprocedural/Exit function test 

#include <stdlib.h>

int optind;
char* optarg;


void
usage (int status)
{

   exit (status);
}

void foo()
{
	usage(1);
	static_assert(0);
}
