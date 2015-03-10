//Coreutils hostname

#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

#define stderr NULL
#define stdout NULL

#define HELP_OPTION_DESCRIPTION \
  _("      --help     display this help and exit\n")
#define VERSION_OPTION_DESCRIPTION \
  _("      --version  output version information and exit\n")
  
  /* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "hostname"
#define PACKAGE_NAME "hostname"
#define AUTHORS "hostname"
#define Version "1.0"
#include <stdlib.h>

#define HAVE_SETHOSTNAME " "

/* Exit statuses for programs like 'env' that exec other programs.  */
enum
{
  EXIT_CANNOT_INVOKE = 126,
  EXIT_ENOENT = 127
};

int errno;
int optind;
char* optarg;

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

void
error (int status, int errnum, const char *message, ...)
{
	if(status!=0) exit(1);
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
		return optarg[0]; /* RETURN POINT 3 */
	}
	return -1; /* RETURN POINT 4 */

}

const char *program_name = "hostname";

void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      printf (_("\
Usage: %s [NAME]\n\
  or:  %s OPTION\n\
Print or set the hostname of the current system.\n\
\n\
"),
             program_name, program_name);
      fputs (HELP_OPTION_DESCRIPTION, stdout);
      fputs (VERSION_OPTION_DESCRIPTION, stdout);
      emit_bug_reporting_address ();
    }
  exit (status);
}



int
main (int argc, char **argv)
{
 assume_size(argv, argc);
 assume(argv!=NULL);

  char *hostname;



  parse_long_options (argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version,
		      usage, AUTHORS, (char const *) NULL);
  if (getopt_long (argc, argv, "", NULL, NULL) != -1)
    usage (EXIT_FAILURE);

  if (argc == optind + 1)
    {
    
#ifdef HAVE_SETHOSTNAME
      /* Set hostname to operand.  */
      buffer_safe(argv, optind);
      char const *name = argv[optind];
      if (sethostname (name, strlen (name)) != 0)
	error (EXIT_FAILURE, errno, _("cannot set name to %s"), quote (name));
#else
      error (EXIT_FAILURE, 0,
	     _("cannot set hostname; this system lacks the functionality"));
#endif
    }

  if (argc <= optind)
    {
      hostname = xgethostname ();
      if (hostname == NULL)
	error (EXIT_FAILURE, errno, _("cannot determine hostname"));
	static_assert(hostname!=NULL);
      printf ("%s\n", hostname);
    }

  if (optind + 1 < argc)
    {
      buffer_safe(argv, optind+1);
      error (0, 0, _("extra operand %s"), quote (argv[optind + 1]));
      usage (EXIT_FAILURE);
    }

  exit (EXIT_SUCCESS);
}