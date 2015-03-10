// Coreutils chroot.c - buggy 2

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

#define	ENOENT		 2	/* No such file or directory */


/* Exit statuses for programs like 'env' that exec other programs.  */
enum
{
  EXIT_CANNOT_INVOKE = 126,
  EXIT_ENOENT = 127
};

int errno;

#define HELP_OPTION_DESCRIPTION  "      --help     display this help and exit\n"
#define VERSION_OPTION_DESCRIPTION "      --version  output version information and exit\n"

void
parse_long_options (int argc,
		    char **argv,
		    const char *command_name,
		    const char *package,
		    const char *version,
		    void (*usage_func) (int),
		    /* const char *author1, ...*/ ...)
{
  optind = 0;
}


int skip(char* s)
{
	return s[0]!='-';
}


int
getopt_long (int argc, char **argv,  char *options,
	     struct option *long_options, int *opt_index)
{
	if(argc < 1) return -1; /* RETURN POINT 1 */
	if(optind == 0) optind = 1;
	
	while( skip(argv[optind]) && optind<argc)
	{
		optind++;
	}
	if(optind>=argc) return -1; /* RETURN POINT 2 */
	optind++;
	if(str_prefix(options, argv[optind]))
	{
		optarg = argv[optind];
		return 0; /* RETURN POINT 3 */
	}
	return -1; /* RETURN POINT 4 */

}

void
usage (int status)
{
	if (status != EXIT_SUCCESS)
		printf ("Try `%s --help' for more information.\n",program_name);
	else
	{
		printf ("Usage: %s NEWROOT [COMMAND [ARG]...]\n\
		or:  %s OPTION\n", program_name, program_name);

		fputs ("Run COMMAND with root directory set to NEWROOT.\n\n", stdout);
		fputs (HELP_OPTION_DESCRIPTION, stdout);
		fputs (VERSION_OPTION_DESCRIPTION, stdout);
		fputs ("\nIf no command is given, run ``${SHELL} -i'' (default: /bin/sh).\n", stdout);
		emit_bug_reporting_address ();
	}
	exit (status);
}

int
main (int argc, char **argv)
{
	//standard system assumptions
	assume(argv != NULL);
	assume_size(argv, argc);
		
	parse_long_options (argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,usage);
	
	
	if (getopt_long (argc, argv, "+", NULL, NULL) != -1){
		usage (EXIT_FAILURE);
	}
	
	/* If getopt_long returns -1, we know that getopt_long
	 * could only have returned at returns points 1, 2, and 4 but not 3.
	 */
  


	/*
	 * If getopt_long returned at return point 1, argc < 1 and optind = 0,
	 * thus this condition holds and we exit.
	 * Similarly, if it retunred at return point 2, optind >=2, so again
	 * the condition would hold and we would exit.
	 */
	if (argc <= optind)  // COND 1
	{
		error (0, 0, "missing operand");
		usage (EXIT_FAILURE);
	}
	
	/*
	 * Hence, if we got down here, we know getopt_long would have returned
	 * at return point 4. If getopt_long returns at point 4, we know
	 * that optind >=2.
	 */

	if (chdir ("/"))
		error (EXIT_FAILURE, "cannot chdir to root directory");

	/*
	 * Since optind >=2, argc >=3, thus all buffer accesses are safe.
	 */
	if (argc == optind + 1) /* COND 2 */
	{
		char *shell = getenv ("SHELL");
		if (shell == NULL)
			shell = bad_cast ("/bin/sh");
		buffer_safe(argv, 0);
		argv[0] = shell;
		buffer_safe(argv, 1);
		argv[1] = bad_cast ("-i");
		buffer_safe(argv, 2);
		argv[2] = NULL;
	}
	
	/*
	 * From COND 1, we know that argc > optind and from COND 2, we know argc != optind +1,
	 * hence, argc > optind +1.
	 */ 
	else
	{
		/* The following arguments give the command.  */
		argv += optind + 2;
	}
	
	/*
	 * If we hit the if case, we didn't do pointer arihmetic and already proved access to 0'th element safe.
	 * If we hit the else case, we incremented the buffer by optind+1, but argc is at least optind+2, so
	 * it's still safe to access the 0'th element.
	 */
	
	/* Execute the given command.  */
	buffer_safe(argv, 0);
	execvp (argv[0], argv);

	{
		int exit_status = (errno == ENOENT ? EXIT_ENOENT : EXIT_CANNOT_INVOKE);
		error (0, errno, _("cannot run command %s"), quote (argv[0]));
		exit (exit_status);
  	}

}
