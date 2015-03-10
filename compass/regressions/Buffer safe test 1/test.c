// Buffer safe test 1

#include "stdlib.h"


int main(int argc, char** argv)
{
	assume(argv != NULL);
	assume_size(argv, argc);
	assume(argc > 1);
	argv +=1;
	buffer_safe(argv, 0);
	
}
