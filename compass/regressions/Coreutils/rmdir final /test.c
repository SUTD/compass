// Coreutils/rmdir final 


/* rmdir -- remove directories

   Copyright (C) 90, 91, 1995-2002, 2004-2008 Free Software
   Foundation, Inc.

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

/* Options:
   -p, --parent		Remove any parent dirs that are explicitly mentioned
			in an argument, if they become empty after the
			argument file is removed.

   David MacKenzie <djm@ai.mit.edu>  */

#include <stdlib.h>


typedef unsigned int bool;
#define true 1
#define false 0

bool verbose = false;
#define stdout NULL
#define stderr NULL

int x;
int errno;

#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

# define AT_FDCWD (-3041965)

#define HELP_OPTION_DESCRIPTION \
  _("      --help     display this help and exit\n")
  
  #define VERSION_OPTION_DESCRIPTION \
  _("      --version  output version information and exit\n")
  
  
  #define IGNORE_FAIL_ON_NON_EMPTY_OPTION 55


#define no_argument false


const char *program_name = "rmdir";

struct option
{
#if defined (__STDC__) && __STDC__
  const char *name;
#else
  char *name;
#endif
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
};

/* Factor out some of the common --help and --version processing code.  */

/* These enum values cannot possibly conflict with the option values
   ordinarily used by commands, including CHAR_MAX + 1, etc.  Avoid
   CHAR_MIN - 1, as it may equal -1, the getopt end-of-options value.  */
  
  
  
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


static struct option const longopts[] =
{
  /* Don't name this `--force' because it's not close enough in meaning
     to e.g. rm's -f option.  */
  {"ignore-fail-on-non-empty", false, NULL,
   IGNORE_FAIL_ON_NON_EMPTY_OPTION},

  {"path", no_argument, NULL, 'p'},  /* Deprecated.  */
  {"parents", no_argument, NULL, 'p'},
  {"verbose", no_argument, NULL, 'v'},
  {GETOPT_HELP_OPTION_DECL},
  {GETOPT_VERSION_OPTION_DECL},
  {NULL, 0, NULL, 0}
};




//--------------------------------------
//Library stubs

char* strrchr(char* s, char c)
{
	assume(x>=0);
	assume(x<buffer_size(s)-1);
	buffer_safe(s, x+1);
	if(unknown()) return NULL;
	return &s[x];
}



int skip(char* s)
{
	return s[0]!='-';
}


int errno;
int opterr;
int optind = 0;
char* optarg;

int getopt_long (int argc, char **argv,  char *options,
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


//---------------------------------------
/* If true, don't treat failure to remove a nonempty directory
   as an error.  */
static bool ignore_fail_on_non_empty;

/* Return true if when rmdir fails with errno == ERROR_NUMBER
   the directory may be empty.  */
static bool
errno_may_be_empty (int error_number)
{
  switch (error_number)
    {
    case EACCES:
    case EPERM:
    case EROFS:
    case EEXIST:
    case EBUSY:
      return true;
    default:
      return false;
    }
}

/* Return true if an rmdir failure with errno == error_number
   for DIR is ignorable.  */
static bool
ignorable_failure (int error_number, char const *dir)
{
  return (ignore_fail_on_non_empty
	  && (errno_rmdir_non_empty (error_number)
	      || (errno_may_be_empty (error_number)
		  && is_empty_dir (AT_FDCWD, dir))));
}

/* Remove any empty parent directories of DIR.
   If DIR contains slash characters, at least one of them
   (beginning with the rightmost) is replaced with a NUL byte.
   Return true if successful.  */

static bool
remove_parents (char *dir)
{
  char *slash;
  bool ok = true;

  strip_trailing_slashes (dir);
  while (1)
    {
      slash = strrchr (dir, '/');
      if (slash == NULL)
	break;
      /* Remove any characters after the slash, skipping any extra
	 slashes in a row. */
      while (slash > dir && *slash == '/')
	--slash;
	buffer_safe(slash, 1);	
	slash[1] = 0;
	
      // Give a diagnostic for each attempted removal if --verbose.  
      if (verbose)
	prog_fprintf (stdout, _("removing directory, %s"), quote (dir));

      ok = (rmdir (dir) == 0);

      if (!ok)
	{
	  // Stop quietly if --ignore-fail-on-non-empty. 
	  if (ignorable_failure (errno, dir))
	    {
	      ok = true;
	    }
	  else
	    {
	      // Barring race conditions, DIR is expected to be a directory.  
	      error (0, errno, _("failed to remove directory %s"),
		     quote (dir));
	    }
	  break;
	}
    }
  return ok;
}

void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      printf (_("Usage: %s [OPTION]... DIRECTORY...\n"), program_name);
      fputs (_("\
Remove the DIRECTORY(ies), if they are empty.\n\
\n\
      --ignore-fail-on-non-empty\n\
                  ignore each failure that is solely because a directory\n\
                    is non-empty\n\
"), stdout);
      fputs (_("\
  -p, --parents   remove DIRECTORY and its ancestors; e.g., `rmdir -p a/b/c' is\n\
                    similar to `rmdir a/b/c a/b a'\n\
  -v, --verbose   output a diagnostic for every directory processed\n\
"), stdout);
      fputs (HELP_OPTION_DESCRIPTION, stdout);
      fputs (VERSION_OPTION_DESCRIPTION, stdout);
      emit_bug_reporting_address ();
    }
  exit (status);
}

bool remove_empty_parents;



#define GETOPT_HELP_CHAR 44

#define PROGRAM_NAME "rmdir"
#define AUTHORS "rmdir"

#define case_GETOPT_HELP_CHAR			\
  case GETOPT_HELP_CHAR:			\
    usage (EXIT_SUCCESS);			\
    break;




/* If true, don't treat failure to remove a nonempty directory
   as an error.  */
static bool ignore_fail_on_non_empty;

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

#define CHAR_MAX 68




int
main (int argc, char **argv)
{
 assume_size(argv, argc);
  bool ok = true;
  int optc;

  remove_empty_parents = false;
  /*while(1)
  {
  	if(optc = getopt_long (argc, argv, "pv", NULL, NULL) != -1)
  	break;
  }*/
 
  while ((optc = getopt_long (argc, argv, "pv", NULL, NULL)) != -1)
   {
    
      switch (optc)
	{
	case 'p':
	  remove_empty_parents = true;
	  break;
	case IGNORE_FAIL_ON_NON_EMPTY_OPTION:
	  ignore_fail_on_non_empty = true;
	  break;
	case 'v':
	  verbose = true;
	  break;
	//case_GETOPT_HELP_CHAR;
	default:
	  usage (EXIT_FAILURE);
	}
    }
     

    if (optind == argc)
    {
      error (0, 0, _("missing operand"));
      usage (EXIT_FAILURE);
    }
      
  
    for (; optind < argc; ++optind)
    {
    
	static_assert(optind<argc);
        buffer_safe(argv, optind);
      char *dir = argv[optind];

     //Give a diagnostic for each attempted removal if --verbose.  
      if (verbose)
	prog_fprintf (stdout, _("removing directory, %s"), quote (dir));

      if (rmdir (dir) != 0)
	{
	  if (ignorable_failure (errno, dir))
	    continue;


	  //Here, the diagnostic is less precise, since we have no idea
	  //   whether DIR is a directory.  
	  error (0, errno, _("failed to remove %s"), quote (dir));
	  ok = false;
	}
      else if (remove_empty_parents)
	{
	  ok &= remove_parents (dir);
	}
    }

  exit (ok ? EXIT_SUCCESS : EXIT_FAILURE);
  
    
}