// Intraprocedural/test assume size 1e

#include <stdlib.h>

int main(int argc, char** argv)
{
	assume_size(argv, argc);
	assume(argc>=1);
	assume(argv!=NULL);
	static_assert(buffer_size(argv)>=1*sizeof(char*));
	
}
