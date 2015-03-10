// Coreutils chroot.c - final full



/* chroot -- run command or shell with special root directory
   Copyright (C) 95, 96, 1997, 1999-2004, 2007-2009
   Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Roland McGrath.  */




#include <stdlib.h>
struct option
{

};

struct va_list
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
int opterr;



/* These are the possibilities for the first argument to setlocale.
   The code assumes that the lowest LC_* symbol has the value zero.  */
#define LC_CTYPE          __LC_CTYPE
#define LC_NUMERIC        __LC_NUMERIC
#define LC_TIME           __LC_TIME
#define LC_COLLATE        __LC_COLLATE
#define LC_MONETARY       __LC_MONETARY
#define LC_MESSAGES       __LC_MESSAGES
#define	LC_ALL		  __LC_ALL
#define LC_PAPER	  __LC_PAPER
#define LC_NAME		  __LC_NAME
#define LC_ADDRESS	  __LC_ADDRESS
#define LC_TELEPHONE	  __LC_TELEPHONE
#define LC_MEASUREMENT	  __LC_MEASUREMENT
#define LC_IDENTIFICATION __LC_IDENTIFICATION


#define HELP_OPTION_DESCRIPTION  "      --help     display this help and exit\n"
#define VERSION_OPTION_DESCRIPTION "      --version  output version information and exit\n"

/* Factor out some of the common --help and --version processing code.  */

#ifndef _BITS_LOCALE_H
#define _BITS_LOCALE_H	1
#endif

enum
{
  __LC_CTYPE = 0,
  __LC_NUMERIC = 1,
  __LC_TIME = 2,
  __LC_COLLATE = 3,
  __LC_MONETARY = 4,
  __LC_MESSAGES = 5,
  __LC_ALL = 6,
  __LC_PAPER = 7,
  __LC_NAME = 8,
  __LC_ADDRESS = 9,
  __LC_TELEPHONE = 10,
  __LC_MEASUREMENT = 11,
  __LC_IDENTIFICATION = 12
};

/* Number of bits in a `char'.  */
#undef CHAR_BIT
#define CHAR_BIT __CHAR_BIT__

/* Maximum length of a multibyte character.  */
#ifndef MB_LEN_MAX
#define MB_LEN_MAX 1
#endif

/* Minimum and maximum values a `signed char' can hold.  */
#undef SCHAR_MIN
#define SCHAR_MIN (-SCHAR_MAX - 1)
#undef SCHAR_MAX
#define SCHAR_MAX __SCHAR_MAX__

/* Maximum value an `unsigned char' can hold.  (Minimum is 0).  */
#undef UCHAR_MAX
#if __SCHAR_MAX__ == __INT_MAX__
# define UCHAR_MAX (SCHAR_MAX * 2U + 1U)
#else
# define UCHAR_MAX (SCHAR_MAX * 2 + 1)
#endif

/* Minimum and maximum values a `char' can hold.  */
#ifdef __CHAR_UNSIGNED__
# undef CHAR_MIN
# if __SCHAR_MAX__ == __INT_MAX__
#  define CHAR_MIN 0U
# else
#  define CHAR_MIN 0
# endif
# undef CHAR_MAX
# define CHAR_MAX UCHAR_MAX
#else
# undef CHAR_MIN
# define CHAR_MIN SCHAR_MIN
# undef CHAR_MAX
# define CHAR_MAX SCHAR_MAX
#endif


/* These enum values cannot possibly conflict with the option values
   ordinarily used by commands, including CHAR_MAX + 1, etc.  Avoid
   CHAR_MIN - 1, as it may equal -1, the getopt end-of-options value.  */
enum
{
  GETOPT_HELP_CHAR = (CHAR_MIN - 2),
  GETOPT_VERSION_CHAR = (CHAR_MIN - 3)
};



#define GETOPT_HELP_OPTION_DECL \
  "help", no_argument, NULL, GETOPT_HELP_CHAR
#define GETOPT_VERSION_OPTION_DECL \
  "version", no_argument, NULL, GETOPT_VERSION_CHAR
#define GETOPT_SELINUX_CONTEXT_OPTION_DECL \
  "context", required_argument, NULL, 'Z'

#define case_GETOPT_HELP_CHAR			\
  case GETOPT_HELP_CHAR:			\
    usage (EXIT_SUCCESS);			\
    break;

/* Program_name must be a literal string.


/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

void
parse_long_options (int argc,
		    char **argv,
		    const char *command_name,
		    const char *package,
		    const char *version,
		    void (*usage_func) (int),
		    /* const char *author1, ...*/ ...)
{
  int c;
  int saved_opterr;

  saved_opterr = opterr;

  /* Don't print an error message for unrecognized options.  */
  opterr = 0;

  if (argc == 2
      && (c = getopt_long (argc, argv, "+",NULL, NULL)) != -1)
    {
      switch (c)
	{
	case 'h':
	  (*usage_func) (EXIT_SUCCESS);

	case 'v':
	  {
	    struct va_list authors;
	    va_start (authors, usage_func);
	    version_etc_va (stdout, command_name, package, version, "");
	    exit (0);
	  }

	default:
	  /* Don't process any other long-named options.  */
	  break;
	}
    }

  /* Restore previous value.  */
  opterr = saved_opterr;

  /* Reset this to zero so that getopt internals get initialized from
     the probably-new parameters when/if getopt is called later.  */
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
#define LOCALEDIR "/"

int
main (int argc, char **argv)
{
	//standard system assumptions
	assume(argv != NULL);
	assume_size(argv, argc);
	
	 initialize_main (&argc, &argv);
	  set_program_name (argv[0]);
	  setlocale (LC_ALL, "");
	  bindtextdomain (PACKAGE, LOCALEDIR);
	  textdomain (PACKAGE);

	  initialize_exit_failure (EXIT_FAILURE);


		
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
		argv += optind + 1;
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
