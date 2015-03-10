// Buffer safe test 1 buggy

#include "stdlib.h"


int main(int argc, char** argv)
{
	assume(argv != NULL);
	assume_size(argv, argc);
	assume(argc > 1);
	argv +=1;
	// should fail
	buffer_safe(argv, 1);
	
}

