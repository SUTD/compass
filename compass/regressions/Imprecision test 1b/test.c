// Imprecision test 1b

#include <stdlib.h>
struct option
{

};

int optind;
char* optarg;
char* program_name = "chroot";
#define PROGRAM_NAME "chroot"
#define PACKAGE_NAME "chroot"
#define Version "2.7"
#define PACKAGE_BUGREPORT "chroot"
#define PACKAGE "chroot"
#define LC_MESSAGES 5

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define stdout NULL


#define HELP_OPTION_DESCRIPTION  "      --help     display this help and exit\n"
#define VERSION_OPTION_DESCRIPTION "      --version  output version information and exit\n"


int skip(char* s)
{
	return s[0]!='-';
}


int
getopt_long (int argc, char **argv,  char *options,
	     struct option *long_options, int *opt_index)
{
	if(argc < 1) return -1;
	if(optind == 0) optind = 1;
	
	while( skip(argv[optind]) && optind<argc)
	{
		optind++;
	}
	if(optind>=argc) return -1;
	optind++;
	return 0;


}


int
main (int argc, char **argv)
{
	int x=-1;
	int y= -1;
	optind = 0;
  	if (getopt_long (argc, argv, "+", NULL, NULL) != -1)
		  exit (EXIT_FAILURE);
	
	x=0;
 	if (argc <= optind) {crap();};
	y=0;
	static_assert(x==y);
	
	

}