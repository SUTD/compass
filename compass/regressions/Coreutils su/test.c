// Coreutils su



/* su for GNU.  Run a shell with substitute user and group IDs.
   Copyright (C) 1992-2006, 2008 Free Software Foundation, Inc.

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

/* Run a shell with the real and effective UID and GID and groups
   of USER, default `root'.

   The shell run is taken from USER's password entry, /bin/sh if
   none is specified there.  If the account has a password, su
   prompts for a password unless run by a user with real UID 0.

   Does not change the current directory.
   Sets `HOME' and `SHELL' from the password entry for USER, and if
   USER is not root, sets `USER' and `LOGNAME' to USER.
   The subshell is not a login shell.

   If one or more ARGs are given, they are passed as additional
   arguments to the subshell.

   Does not handle /bin/sh or other shells specially
   (setting argv[0] to "-su", passing -c only to certain shells, etc.).
   I don't see the point in doing that, and it's ugly.

   This program intentionally does not support a "wheel group" that
   restricts who can su to UID 0 accounts.  RMS considers that to
   be fascist.

   Compile-time options:
   -DSYSLOG_SUCCESS	Log successful su's (by default, to root) with syslog.
   -DSYSLOG_FAILURE	Log failed su's (by default, to root) with syslog.

   -DSYSLOG_NON_ROOT	Log all su's, not just those to root (UID 0).
   Never logs attempted su's to nonexistent accounts.

   Written by David MacKenzie <djm@gnu.ai.mit.edu>.  */
   
   
   
   
   

#include <stdlib.h>

//typedef unsigned int size_t;
typedef unsigned int bool;

/* The passwd structure.  */
struct passwd
{
  char *pw_name;		/* Username.  */
  char *pw_passwd;		/* Password.  */
  int pw_uid;		/* User ID.  */
  int pw_gid;		/* Group ID.  */
  char *pw_gecos;		/* Real name.  */
  char *pw_dir;			/* Home directory.  */
  char *pw_shell;		/* Shell program.  */
};

/* Structure giving information about numeric and monetary notation.  */
struct lconv
{
  /* Numeric (non-monetary) information.  */

  char *decimal_point;		/* Decimal point character.  */
  char *thousands_sep;		/* Thousands separator.  */
  /* Each element is the number of digits in each group;
     elements with higher indices are farther left.
     An element with value CHAR_MAX means that no further grouping is done.
     An element with value 0 means that the previous element is used
     for all groups farther left.  */
  char *grouping;

  /* Monetary information.  */

  /* First three chars are a currency symbol from ISO 4217.
     Fourth char is the separator.  Fifth char is '\0'.  */
  char *int_curr_symbol;
  char *currency_symbol;	/* Local currency symbol.  */
  char *mon_decimal_point;	/* Decimal point character.  */
  char *mon_thousands_sep;	/* Thousands separator.  */
  char *mon_grouping;		/* Like `grouping' element (above).  */
  char *positive_sign;		/* Sign for positive values.  */
  char *negative_sign;		/* Sign for negative values.  */
  char int_frac_digits;		/* Int'l fractional digits.  */
  char frac_digits;		/* Local fractional digits.  */
  /* 1 if currency_symbol precedes a positive value, 0 if succeeds.  */
  char p_cs_precedes;
  /* 1 iff a space separates currency_symbol from a positive value.  */
  char p_sep_by_space;
  /* 1 if currency_symbol precedes a negative value, 0 if succeeds.  */
  char n_cs_precedes;
  /* 1 iff a space separates currency_symbol from a negative value.  */
  char n_sep_by_space;
  /* Positive and negative sign positions:
     0 Parentheses surround the quantity and currency_symbol.
     1 The sign string precedes the quantity and currency_symbol.
     2 The sign string follows the quantity and currency_symbol.
     3 The sign string immediately precedes the currency_symbol.
     4 The sign string immediately follows the currency_symbol.  */
  char p_sign_posn;
  char n_sign_posn;
#ifdef __USE_ISOC99
  /* 1 if int_curr_symbol precedes a positive value, 0 if succeeds.  */
  char int_p_cs_precedes;
  /* 1 iff a space separates int_curr_symbol from a positive value.  */
  char int_p_sep_by_space;
  /* 1 if int_curr_symbol precedes a negative value, 0 if succeeds.  */
  char int_n_cs_precedes;
  /* 1 iff a space separates int_curr_symbol from a negative value.  */
  char int_n_sep_by_space;
  /* Positive and negative sign positions:
     0 Parentheses surround the quantity and int_curr_symbol.
     1 The sign string precedes the quantity and int_curr_symbol.
     2 The sign string follows the quantity and int_curr_symbol.
     3 The sign string immediately precedes the int_curr_symbol.
     4 The sign string immediately follows the int_curr_symbol.  */
  char int_p_sign_posn;
  char int_n_sign_posn;
#else
  char __int_p_cs_precedes;
  char __int_p_sep_by_space;
  char __int_n_cs_precedes;
  char __int_n_sep_by_space;
  char __int_p_sign_posn;
  char __int_n_sign_posn;
#endif
};

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

#define Version "2.0"






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

#define	LC_ALL		  __LC_ALL

#define true 1
#define false 0

#define CHAR_MAX 128
#define CHAR_MIN 0

#define stderr NULL
#define stdout NULL
char* program_name = "su";

/* The user to become if none is specified.  */
#define DEFAULT_USER "root"

#define PACKAGE "su"

#define LOCALEDIR "/"

#define PACKAGE_NAME "su"

static bool ignore_EPIPE /* = false */;




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
    
    

/* These enum values cannot possibly conflict with the option values
   ordinarily used by commands, including CHAR_MAX + 1, etc.  Avoid
   CHAR_MIN - 1, as it may equal -1, the getopt end-of-options value.  */
enum
{
  GETOPT_HELP_CHAR = (CHAR_MIN - 2),
  GETOPT_VERSION_CHAR = (CHAR_MIN - 3)
};

/* Exit statuses for programs like 'env' that exec other programs.  */
enum
{
  EXIT_CANNOT_INVOKE = 126,
  EXIT_ENOENT = 127
};

#define HELP_OPTION_DESCRIPTION (" --help     display this help and exit\n")

#define VERSION_OPTION_DESCRIPTION (" --version  output version information and exit\n")


#define	ENOENT		 2	/* No such file or directory */


#define required_argument 2

#define optional_argument 1

#define no_argument 0

#define GETOPT_HELP_OPTION_DECL \
  "help", no_argument, NULL, GETOPT_HELP_CHAR
  
  #define GETOPT_VERSION_OPTION_DECL \
  "version", no_argument, NULL, GETOPT_VERSION_CHAR
  
  #define case_GETOPT_HELP_CHAR			\
  case GETOPT_HELP_CHAR:			\
    usage (EXIT_SUCCESS);			\
    break;
    
    #define case_GETOPT_VERSION_CHAR(Program_name, Authors)			\
  case GETOPT_VERSION_CHAR:						\
    version_etc (stdout, Program_name, PACKAGE_NAME, Version, Authors,	\
                 (char *) NULL);					\
    exit (EXIT_SUCCESS);						\
    break;

/* Hide any system prototype for getusershell.
   This is necessary because some Cray systems have a conflicting
   prototype (returning `int') in <unistd.h>.  */
#define getusershell _getusershell_sys_proto_

static const char *file_name;



/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "su"

#define AUTHORS proper_name ("David MacKenzie")

int errno;

#define stdout NULL


/* The default PATH for simulated logins to non-superuser accounts.  */
#ifdef _PATH_DEFPATH
# define DEFAULT_LOGIN_PATH _PATH_DEFPATH
#else
# define DEFAULT_LOGIN_PATH ":/usr/ucb:/bin:/usr/bin"
#endif

/* The default PATH for simulated logins to superuser accounts.  */
#ifdef _PATH_DEFPATH_ROOT
# define DEFAULT_ROOT_LOGIN_PATH _PATH_DEFPATH_ROOT
#else
# define DEFAULT_ROOT_LOGIN_PATH "/usr/ucb:/bin:/usr/bin:/etc"
#endif

/* The shell to run if none is given in the user's passwd entry.  */
#define DEFAULT_SHELL "/bin/sh"

/* The user to become if none is specified.  */
#define DEFAULT_USER "root"

#define	EPIPE		32	/* Broken pipe */

struct option
{
  const char *name;
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
};

char *crypt (char const *key, char const *salt);
char *getusershell (void);
void endusershell (void);
void setusershell (void);

extern char **environ;

static void run_shell (char const *, char const *, char **, size_t);
    // ATTRIBUTE_NORETURN;

/* If true, pass the `-f' option to the subshell.  */
static bool fast_startup;

/* If true, simulate a login instead of just starting a shell.  */
static bool simulate_login;

/* If true, change some environment vars to indicate the user su'd to.  */
static bool change_environment;

/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

int volatile exit_failure = EXIT_FAILURE;

#  define DIRECTORY_SEPARATOR '/'
#  define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)


#if defined SYSLOG_SUCCESS || defined SYSLOG_FAILURE
/* Log the fact that someone has run su to the user given by PW;
   if SUCCESSFUL is true, they gave the correct password, etc.  */

static void
log_su (struct passwd const *pw, bool successful)
{
  const char *new_user, *old_user, *tty;

# ifndef SYSLOG_NON_ROOT
  if (pw->pw_uid)
    return;
# endif
  new_user = pw->pw_name;
  /* The utmp entry (via getlogin) is probably the best way to identify
     the user, especially if someone su's from a su-shell.  */
  old_user = getlogin ();
  if (!old_user)
    {
      /* getlogin can fail -- usually due to lack of utmp entry.
	 Resort to getpwuid.  */
      struct passwd *pwd = getpwuid (getuid ());
      old_user = (pwd ? pwd->pw_name : "");
    }
  tty = ttyname (STDERR_FILENO);
  if (!tty)
    tty = "none";
  /* 4.2BSD openlog doesn't have the third parameter.  */
  openlog (last_component (program_name), 0
# ifdef LOG_AUTH
	   , LOG_AUTH
# endif
	   );
  syslog (LOG_NOTICE,
# ifdef SYSLOG_NON_ROOT
	  "%s(to %s) %s on %s",
# else
	  "%s%s on %s",
# endif
	  successful ? "" : "FAILED SU ",
# ifdef SYSLOG_NON_ROOT
	  new_user,
# endif
	  old_user, tty);
  closelog ();
}
#endif

#if defined SYSLOG_SUCCESS || defined SYSLOG_FAILURE
/* Log the fact that someone has run su to the user given by PW;
   if SUCCESSFUL is true, they gave the correct password, etc.  */

static void
log_su (struct passwd const *pw, bool successful)
{
  const char *new_user, *old_user, *tty;

# ifndef SYSLOG_NON_ROOT
  if (pw->pw_uid)
    return;
# endif
  new_user = pw->pw_name;
  /* The utmp entry (via getlogin) is probably the best way to identify
     the user, especially if someone su's from a su-shell.  */
  old_user = getlogin ();
  if (!old_user)
    {
      /* getlogin can fail -- usually due to lack of utmp entry.
	 Resort to getpwuid.  */
      struct passwd *pwd = getpwuid (getuid ());
      old_user = (pwd ? pwd->pw_name : "");
    }
  tty = ttyname (STDERR_FILENO);
  if (!tty)
    tty = "none";
  /* 4.2BSD openlog doesn't have the third parameter.  */
  openlog (last_component (program_name), 0
# ifdef LOG_AUTH
	   , LOG_AUTH
# endif
	   );
  syslog (LOG_NOTICE,
# ifdef SYSLOG_NON_ROOT
	  "%s(to %s) %s on %s",
# else
	  "%s%s on %s",
# endif
	  successful ? "" : "FAILED SU ",
# ifdef SYSLOG_NON_ROOT
	  new_user,
# endif
	  old_user, tty);
  closelog ();
}
#endif

static struct option const longopts[] =
{
  {"command", required_argument, NULL, 'c'},
  {"fast", no_argument, NULL, 'f'},
  {"login", no_argument, NULL, 'l'},
  {"preserve-environment", no_argument, NULL, 'p'},
  {"shell", required_argument, NULL, 's'},
  {GETOPT_HELP_OPTION_DECL},
  {GETOPT_VERSION_OPTION_DECL},
  {NULL, 0, NULL, 0}
};

/* Close standard output.  On error, issue a diagnostic and _exit
   with status 'exit_failure'.

   Also close standard error.  On error, _exit with status 'exit_failure'.

   Since close_stdout is commonly registered via 'atexit', POSIX
   and the C standard both say that it should not call 'exit',
   because the behavior is undefined if 'exit' is called more than
   once.  So it calls '_exit' instead of 'exit'.  If close_stdout
   is registered via atexit before other functions are registered,
   the other functions can act before this _exit is invoked.

   Applications that use close_stdout should flush any streams
   other than stdout and stderr before exiting, since the call to
   _exit will bypass other buffer flushing.  Applications should
   be flushing and closing other streams anyway, to check for I/O
   errors.  Also, applications should not use tmpfile, since _exit
   can bypass the removal of these files.

   It's important to detect such failures and exit nonzero because many
   tools (most notably `make' and other build-management systems) depend
   on being able to detect failure in other tools via their exit status.  */

void
close_stdout (void)
{
  if (close_stream (stdout) != 0
      && !(ignore_EPIPE && errno == EPIPE))
    {
      char const *write_error = _("write error");
      if (file_name)
	error (0, errno, "%s: %s", quotearg_colon (file_name),
	       write_error);
      else
	error (0, errno, "%s", write_error);

      exit (exit_failure);
    }

   if (close_stream (stderr) != 0)
     exit (exit_failure);
}

#if defined SYSLOG_SUCCESS || defined SYSLOG_FAILURE
/* Log the fact that someone has run su to the user given by PW;
   if SUCCESSFUL is true, they gave the correct password, etc.  */

static void
log_su (struct passwd const *pw, bool successful)
{
  const char *new_user, *old_user, *tty;

# ifndef SYSLOG_NON_ROOT
  if (pw->pw_uid)
    return;
# endif
  new_user = pw->pw_name;
  /* The utmp entry (via getlogin) is probably the best way to identify
     the user, especially if someone su's from a su-shell.  */
  old_user = getlogin ();
  if (!old_user)
    {
      /* getlogin can fail -- usually due to lack of utmp entry.
	 Resort to getpwuid.  */
      struct passwd *pwd = getpwuid (getuid ());
      old_user = (pwd ? pwd->pw_name : "");
    }
  tty = ttyname (STDERR_FILENO);
  if (!tty)
    tty = "none";
  /* 4.2BSD openlog doesn't have the third parameter.  */
  openlog (last_component (program_name), 0
# ifdef LOG_AUTH
	   , LOG_AUTH
# endif
	   );
  syslog (LOG_NOTICE,
# ifdef SYSLOG_NON_ROOT
	  "%s(to %s) %s on %s",
# else
	  "%s%s on %s",
# endif
	  successful ? "" : "FAILED SU ",
# ifdef SYSLOG_NON_ROOT
	  new_user,
# endif
	  old_user, tty);
  closelog ();
}
#endif

int strlen(char* s)
{	
	int i=0;
	for(; s[i]!='\0'; i++);
	return i;
}

/* Add NAME=VAL to the environment, checking for out of memory errors.  */

static void
xsetenv (char const *name, char const *val)
{
  size_t namelen = strlen (name);
  size_t vallen = strlen (val);
  char *string = malloc (namelen + 1 + vallen + 1);
  buffer_safe(string, namelen);
  strcpy (string, name);
  string[namelen] = '=';
  buffer_safe(string+namelen+1, vallen);
  strcpy (string + namelen + 1, val);
  if (putenv (string) != 0)
    xalloc_die ();
}

/* Return the address of the last file name component of NAME.  If
   NAME has no relative file name components because it is a file
   system root, return the empty string.  */

char *
last_component (char const *name)
{
  char const *base = name + FILE_SYSTEM_PREFIX_LEN (name);
  char const *p;
  bool saw_slash = false;

  while (ISSLASH (*base))
    base++;

  for (p = base; *p; p++)
    {
      if (ISSLASH (*p))
	saw_slash = true;
      else if (saw_slash)
	{
	  base = p;
	  saw_slash = false;
	}
    }

  return (char *) base;
}



static void
run_shell (char const *shell, char const *command, char **additional_args,
	   size_t n_additional_args)
{
  size_t n_args = 1 + fast_startup + 2 * !!command + n_additional_args + 1;
  char const **args = malloc (n_args * sizeof *args);
  size_t argno = 1;
     

  if (simulate_login)
    {
      char *arg0;
      char *shell_basename;

      shell_basename = last_component (shell);
      arg0 = malloc (strlen (shell_basename) + 2);
      buffer_safe(arg0, 0);
      arg0[0] = '-';
      buffer_safe(arg0, 1);
      strcpy (arg0 + 1, shell_basename);
      args[0] = arg0;
    }
      else {
      buffer_safe(args, 0);
    args[0] = last_component (shell);
    }
    
      if (fast_startup) {
     buffer_safe(args, argno);
    args[argno++] = "-f";
    }
    
      if (command)
    {
      buffer_safe(args, argno+1);
      args[argno++] = "-c";
      args[argno++] = command;
    }
    if(n_additional_args>0) 
    {
    	buffer_safe(args + argno, n_additional_args-1);
    	buffer_safe(additional_args, n_additional_args-1);
    }
    memcpy ( args + argno, additional_args, n_additional_args * sizeof(args));
    args[argno + n_additional_args] = NULL;
      execv (shell, (char **) args);

  {
    int exit_status = (errno == ENOENT ? EXIT_ENOENT : EXIT_CANNOT_INVOKE);
    error (0, errno, "%s", shell);
    exit (exit_status);
  }
}


/* Return true if SHELL is a restricted shell (one not returned by
   getusershell), else false, meaning it is a standard shell.  */

static bool
restricted_shell (const char *shell)
{
  char *line;

  setusershell ();
  while ((line = getusershell ()) != NULL)
    {
      if (*line != '#' && STREQ (line, shell))
	{
	  endusershell ();
	  return false;
	}
    }
  endusershell ();
  return true;
}



void
usage (int status)
{

  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      printf (_("Usage: %s [OPTION]... [-] [USER [ARG]...]\n"), program_name);
      fputs (_("\
Change the effective user id and group id to that of USER.\n\
\n\
  -, -l, --login               make the shell a login shell\n\
  -c, --command=COMMAND        pass a single COMMAND to the shell with -c\n\
  -f, --fast                   pass -f to the shell (for csh or tcsh)\n\
  -m, --preserve-environment   do not reset environment variables\n\
  -p                           same as -m\n\
  -s, --shell=SHELL            run SHELL if /etc/shells allows it\n\
"), stdout);
      fputs (HELP_OPTION_DESCRIPTION, stdout);
      fputs (VERSION_OPTION_DESCRIPTION, stdout);
      fputs (_("\
\n\
A mere - implies -l.   If USER not given, assume root.\n\
"), stdout);
      emit_bug_reporting_address ();
    }
    
  exit (status);
}



/* Ask the user for a password.
   Return true if the user gives the correct password for entry PW,
   false if not.  Return true without asking for a password if run by UID 0
   or if PW has an empty password.  */

static bool
correct_password (const struct passwd *pw)
{
     static_assert(pw!=NULL);
  char *unencrypted, *encrypted, *correct;
#if HAVE_GETSPNAM && HAVE_STRUCT_SPWD_SP_PWDP

  struct spwd *sp = getspnam (pw->pw_name);

  endspent ();
  if (sp)
    correct = sp->sp_pwdp;
  else
#endif

    correct = pw->pw_passwd;

  if (getuid () == 0 || !correct || correct[0] == '\0')
    return true;

  unencrypted = getpass (_("Password:"));
  if (!unencrypted)
    {
      error (0, 0, _("getpass: cannot open /dev/tty"));
      return false;
    }
  encrypted = crypt (unencrypted, correct);
  buffer_safe(unencrypted, strlen(unencrypted));
  memset (unencrypted, 0, strlen (unencrypted));
  return STREQ (encrypted, correct);
}



/* Update `environ' for the new shell based on PW, with SHELL being
   the value for the SHELL environment variable.  */

static void
modify_environment (const struct passwd *pw, const char *shell)
{
  static_assert(pw!=NULL);
  if (simulate_login)
    {
      /* Leave TERM unchanged.  Set HOME, SHELL, USER, LOGNAME, PATH.
         Unset all other environment variables.  */
      char const *term = getenv ("TERM");
      if (term)
	term = xstrdup (term);
      environ = malloc ((6 + !!term) * sizeof (char *));
      buffer_safe(environ, 0);
      environ[0] = NULL;
      if (term)
	xsetenv ("TERM", term);
      xsetenv ("HOME", pw->pw_dir);
      xsetenv ("SHELL", shell);
      xsetenv ("USER", pw->pw_name);
      xsetenv ("LOGNAME", pw->pw_name);
      xsetenv ("PATH", (pw->pw_uid
			? DEFAULT_LOGIN_PATH
			: DEFAULT_ROOT_LOGIN_PATH));
    }
  else
    {
      /* Set HOME, SHELL, and if not becoming a super-user,
	 USER and LOGNAME.  */
      if (change_environment)
	{
	  xsetenv ("HOME", pw->pw_dir);
	  xsetenv ("SHELL", shell);
	  if (pw->pw_uid)
	    {
	      xsetenv ("USER", pw->pw_name);
	      xsetenv ("LOGNAME", pw->pw_name);
	    }
	}
    }
}





/* Become the user and group(s) specified by PW.  */

static void
change_identity (const struct passwd *pw)
{
 static_assert(pw!=NULL);
#ifdef HAVE_INITGROUPS
  errno = 0;
  if (initgroups (pw->pw_name, pw->pw_gid) == -1)
    error (EXIT_FAILURE, errno, _("cannot set groups"));
  endgrent ();
#endif
  if (setgid (pw->pw_gid))
    error (EXIT_FAILURE, errno, _("cannot set group id"));
  if (setuid (pw->pw_uid))
    error (EXIT_FAILURE, errno, _("cannot set user id"));
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
		return get_first_opt(optarg); 
	}
	return -1; 
}


/* Print the program name and error message MESSAGE, which is a printf-style
   format string with optional args.
   If ERRNUM is nonzero, print its corresponding system error message.
   Exit with status STATUS if it is nonzero.  */
void
error (int status, int errnum, const char *message, ...)
{
	if(status!=0) exit(1);
}

   /*This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern struct passwd *getpwnam (__const char *__name);







int
main (int argc, char **argv)
{
  assume(argv!=NULL);
  assume_size(argv, argc);
  
  int optc;
  const char *new_user = DEFAULT_USER;
  char *command = NULL;
  char *shell = NULL;
  struct passwd *pw;
  struct passwd pw_copy;

  initialize_main (&argc, &argv);
  set_program_name (argv[0]);
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  initialize_exit_failure (EXIT_FAILURE);
  atexit (close_stdout);

  fast_startup = false;
  simulate_login = false;
  change_environment = true;
  
    while ((optc = getopt_long (argc, argv, "c:flmps:", NULL, NULL)) != -1)
    {
      switch (optc)
	{
	case 'c':
	 command = optarg;
	  break;

	case 'f':
	  fast_startup = true;
	  break;

	case 'l':
	  simulate_login = true;
	  break;

	case 'm':
	case 'p':
	  change_environment = false;
	  break;

	case 's':
	  shell = optarg;
	  break;

	case_GETOPT_HELP_CHAR;

	case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);

	default:
	  usage (EXIT_FAILURE);
	}
    }
    
    
     if (optind < argc) 
    {
        buffer_safe(argv, optind);
    	if(STREQ (argv[optind], "-")){
      		simulate_login = true;
      		++optind;
      	}
    }
    
      if (optind < argc) {
      	buffer_safe(argv, optind);
   	 new_user = argv[optind++];
    }
    
  pw = getpwnam (new_user);
  if (! (pw && pw->pw_name && pw->pw_name[0] && pw->pw_dir && pw->pw_dir[0]
	 && pw->pw_passwd))
    error (EXIT_FAILURE, 0, _("user %s does not exist"), new_user);
    
    
      /* Make a copy of the password information and point pw at the local
     copy instead.  Otherwise, some systems (e.g. Linux) would clobber
     the static data through the getlogin call from log_su.
     Also, make sure pw->pw_shell is a nonempty string.
     It may be NULL when NEW_USER is a username that is retrieved via NIS (YP),
     but that doesn't have a default shell listed.  */
   static_assert(pw!=NULL);
  pw_copy = *pw;
  pw = &pw_copy;
  pw->pw_name = xstrdup (pw->pw_name);
  pw->pw_passwd = xstrdup (pw->pw_passwd);
  pw->pw_dir = xstrdup (pw->pw_dir);
  pw->pw_shell = xstrdup (pw->pw_shell && pw->pw_shell[0]
			  ? pw->pw_shell
			  : DEFAULT_SHELL);
  endpwent ();
  
    if (!correct_password (pw))
    {
#ifdef SYSLOG_FAILURE
      log_su (pw, false);
#endif
      error (EXIT_FAILURE, 0, _("incorrect password"));
    }
#ifdef SYSLOG_SUCCESS
  else
    {
      log_su (pw, true);
    }
#endif

  if (!shell && !change_environment)
    shell = getenv ("SHELL");
  if (shell && getuid () != 0 && restricted_shell (pw->pw_shell))
    {
      /* The user being su'd to has a nonstandard shell, and so is
	 probably a uucp account or has restricted access.  Don't
	 compromise the account by allowing access with a standard
	 shell.  */
      error (0, 0, _("using restricted shell %s"), pw->pw_shell);
      shell = NULL;
    }
    
   shell = xstrdup (shell ? shell : pw->pw_shell);
  modify_environment (pw, shell);
  change_identity (pw);
  if (simulate_login && chdir (pw->pw_dir) != 0)
    error (0, errno, _("warning: cannot change directory to %s"), pw->pw_dir);

 int remaining_args = argc - optind;
 if(remaining_args<0) remaining_args = 0; 
  run_shell (shell, command, argv + optind, remaining_args);
  
  }
