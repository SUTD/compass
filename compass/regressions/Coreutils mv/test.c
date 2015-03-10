// Coreutils mv

#include <stdlib.h>

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "mv"

#define false 0
#define true 1


#define CHAR_MAX 128
#define CHAR_MIN 0

#define PACKAGE_NAME "mv"
#define Version 2.7

char* simple_backup_suffix;

typedef int bool;

#define AUTHORS \
  proper_name ("Mike Parker"), \
  proper_name ("David MacKenzie"), \
  proper_name ("Jim Meyering")
  
  
#define LC_ALL " "
#define PACKAGE "mv"
#define LOCALEDIR "/"
char* program_name = "mv";
#define stdout NULL

#define HELP_OPTION_DESCRIPTION \
  _("      --help     display this help and exit\n")
#define VERSION_OPTION_DESCRIPTION \
  _("      --version  output version information and exit\n")


struct option
{
  const char *name;
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
};

enum RM_status
{
  /* These must be listed in order of increasing seriousness. */
  RM_OK = 2,
  RM_USER_DECLINED,
  RM_ERROR,
  RM_NONEMPTY_DIR
};

#define required_argument 2
#define optional_argument 1
#define no_argument 0

#define stderr NULL


/* For long options that have no equivalent short option, use a
   non-character as a pseudo short option, starting with CHAR_MAX + 1.  */
enum
{
  STRIP_TRAILING_SLASHES_OPTION = CHAR_MAX + 1
};

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

/* Remove any trailing slashes from each SOURCE argument.  */
static bool remove_trailing_slashes;

static struct option const long_options[] =
{
  {"backup", optional_argument, NULL, 'b'},
  {"force", no_argument, NULL, 'f'},
  {"interactive", no_argument, NULL, 'i'},
  {"no-clobber", no_argument, NULL, 'n'},
  {"no-target-directory", no_argument, NULL, 'T'},
  {"strip-trailing-slashes", no_argument, NULL, STRIP_TRAILING_SLASHES_OPTION},
  {"suffix", required_argument, NULL, 'S'},
  {"target-directory", required_argument, NULL, 't'},
  {"update", no_argument, NULL, 'u'},
  {"verbose", no_argument, NULL, 'v'},
  {GETOPT_HELP_OPTION_DECL},
  {GETOPT_VERSION_OPTION_DECL},
  {NULL, 0, NULL, 0}
};



struct dev_ino
{
  ino_t st_ino;
  dev_t st_dev;
};

 struct stat
 {
  ino_t st_ino;
  dev_t st_dev;
  int st_mode;
 };



enum rm_interactive
{
  /* Start with any number larger than 1, so that any legacy tests
     against values of 0 or 1 will fail.  */
  RMI_ALWAYS = 3,
  RMI_SOMETIMES,
  RMI_NEVER
};


struct rm_options
{
  /* If true, ignore nonexistent files.  */
  bool ignore_missing_files;

  /* If true, query the user about whether to remove each file.  */
  enum rm_interactive interactive;

  /* If true, do not traverse into (or remove) any directory that is
     on a file system (i.e., that has a different device number) other
     than that of the corresponding command line argument.  Note that
     even without this option, rm will fail in the end, due to its
     probable inability to remove the mount point.  But there, the
     diagnostic comes too late -- after removing all contents.  */
  bool one_file_system;

  /* If true, recursively remove directories.  */
  bool recursive;

  /* Pointer to the device and inode numbers of `/', when --recursive
     and preserving `/'.  Otherwise NULL.  */
  struct dev_ino *root_dev_ino;

  /* If nonzero, stdin is a tty.  */
  bool stdin_tty;

  /* If true, display the name of each file removed.  */
  bool verbose;

  /* If true, treat the failure by the rm function to restore the
     current working directory as a fatal error.  I.e., if this field
     is true and the rm function cannot restore cwd, it must exit with
     a nonzero status.  Some applications require that the rm function
     restore cwd (e.g., mv) and some others do not (e.g., rm,
     in many cases).  */
  bool require_restore_cwd;
};


/* When to make backup files. */
enum backup_type
{
  /* Never make backups. */
  no_backups,

  /* Make simple backups of every file. */
  simple_backups,

  /* Make numbered backups of files that already have numbered backups,
     and simple backups of the others. */
  numbered_existing_backups,

  /* Make numbered backups of every file. */
  numbered_backups
};

/* How to handle symbolic links.  */
enum Dereference_symlink
{
  DEREF_UNDEFINED = 1,

  /* Copy the symbolic link itself.  -P  */
  DEREF_NEVER,

  /* If the symbolic is a command line argument, then copy
     its referent.  Otherwise, copy the symbolic link itself.  -H  */
  DEREF_COMMAND_LINE_ARGUMENTS,

  /* Copy the referent of the symbolic link.  -L  */
  DEREF_ALWAYS
};

enum Sparse_type
{
  SPARSE_UNUSED,

  /* Never create holes in DEST.  */
  SPARSE_NEVER,

  /* This is the default.  Use a crude (and sometimes inaccurate)
     heuristic to determine if SOURCE has holes.  If so, try to create
     holes in DEST.  */
  SPARSE_AUTO,

  /* For every sufficiently long sequence of bytes in SOURCE, try to
     create a corresponding hole in DEST.  There is a performance penalty
     here because CP has to search for holes in SRC.  But if the holes are
     big enough, that penalty can be offset by the decrease in the amount
     of data written to disk.   */
  SPARSE_ALWAYS
};

/* This type is used to help mv (via copy.c) distinguish these cases.  */
enum Interactive
{
  I_ALWAYS_YES = 1,
  I_ALWAYS_NO,
  I_ASK_USER,
  I_UNSPECIFIED
};

struct hash_table;

typedef struct hash_table Hash_table;


/* These options control how files are copied by at least the
   following programs: mv (when rename doesn't work), cp, install.
   So, if you add a new member, be sure to initialize it in
   mv.c, cp.c, and install.c.  */
struct cp_options
{
  enum backup_type backup_type;

  /* How to handle symlinks in the source.  */
  enum Dereference_symlink dereference;

  /* This value is used to determine whether to prompt before removing
     each existing destination file.  It works differently depending on
     whether move_mode is set.  See code/comments in copy.c.  */
  enum Interactive interactive;

  /* Control creation of sparse files.  */
  enum Sparse_type sparse_mode;

  /* Set the mode of the destination file to exactly this value
     if SET_MODE is nonzero.  */
  mode_t mode;

  /* If true, copy all files except (directories and, if not dereferencing
     them, symbolic links,) as if they were regular files.  */
  bool copy_as_regular;

  /* If true, remove each existing destination nondirectory before
     trying to open it.  */
  bool unlink_dest_before_opening;

  /* If true, first try to open each existing destination nondirectory,
     then, if the open fails, unlink and try again.
     This option must be set for `cp -f', in case the destination file
     exists when the open is attempted.  It is irrelevant to `mv' since
     any destination is sure to be removed before the open.  */
  bool unlink_dest_after_failed_open;

  /* If true, create hard links instead of copying files.
     Create destination directories as usual. */
  bool hard_link;

  /* If true, rather than copying, first attempt to use rename.
     If that fails, then resort to copying.  */
  bool move_mode;

  /* Whether this process has appropriate privileges to chown a file
     whose owner is not the effective user ID.  */
  bool chown_privileges;

  /* Whether this process has appropriate privileges to do the
     following operations on a file even when it is owned by some
     other user: set the file's atime, mtime, mode, or ACL; remove or
     rename an entry in the file even though it is a sticky directory,
     or to mount on the file.  */
  bool owner_privileges;

  /* If true, when copying recursively, skip any subdirectories that are
     on different file systems from the one we started on.  */
  bool one_file_system;

  /* If true, attempt to give the copies the original files' permissions,
     owner, group, and timestamps. */
  bool preserve_ownership;
  bool preserve_mode;
  bool preserve_timestamps;

  /* Enabled for mv, and for cp by the --preserve=links option.
     If true, attempt to preserve in the destination files any
     logical hard links between the source files.  If used with cp's
     --no-dereference option, and copying two hard-linked files,
     the two corresponding destination files will also be hard linked.

     If used with cp's --dereference (-L) option, then, as that option implies,
     hard links are *not* preserved.  However, when copying a file F and
     a symlink S to F, the resulting S and F in the destination directory
     will be hard links to the same file (a copy of F).  */
  bool preserve_links;

  /* If true and any of the above (for preserve) file attributes cannot
     be applied to a destination file, treat it as a failure and return
     nonzero immediately.  E.g. for cp -p this must be true, for mv it
     must be false.  */
  bool require_preserve;

  /* If true, attempt to preserve the SELinux security context, too.
     Set this only if the kernel is SELinux enabled.  */
  bool preserve_security_context;

  /* Useful only when preserve_security_context is true.
     If true, a failed attempt to preserve a file's security context
     propagates failure "out" to the caller.  If false, a failure to
     preserve a file's security context does not change the invoking
     application's exit status.  Give diagnostics for failed syscalls
     regardless of this setting.  For example, with "cp --preserve=context"
     this flag is "true", while with "cp -a", it is false.  That means
     "cp -a" attempts to preserve any security context, but does not
     fail if it is unable to do so.  */
  bool require_preserve_context;

  /* If true, attempt to preserve extended attributes using libattr.
     Ignored if coreutils are compiled without xattr support. */
  bool preserve_xattr;

  /* Useful only when preserve_xattr is true.
     If true, a failed attempt to preserve file's extended attributes
     propagates failure "out" to the caller.  If false, a failure to
     preserve file's extended attributes does not change the invoking
     application's exit status.  Give diagnostics for failed syscalls
     regardless of this setting.  For example, with "cp --preserve=xattr"
     this flag is "true", while with "cp --preserve=all", it is false. */
  bool require_preserve_xattr;

  /* Used as difference boolean between cp -a and cp -dR --preserve=all.
     If true, non-mandatory failure diagnostics are not displayed. This
     should prevent poluting cp -a output.
   */
  bool reduce_diagnostics;

  /* If true, copy directories recursively and copy special files
     as themselves rather than copying their contents. */
  bool recursive;

  /* If true, set file mode to value of MODE.  Otherwise,
     set it based on current umask modified by UMASK_KILL.  */
  bool set_mode;

  /* If true, create symbolic links instead of copying files.
     Create destination directories as usual. */
  bool symbolic_link;

  /* If true, do not copy a nondirectory that has an existing destination
     with the same or newer modification time. */
  bool update;

  /* If true, display the names of the files before copying them. */
  bool verbose;

  /* If true, stdin is a tty.  */
  bool stdin_tty;

  /* If true, open a dangling destination symlink when not in move_mode.
     Otherwise, copy_reg gives a diagnostic (it refuses to write through
     such a symlink) and returns false.  */
  bool open_dangling_dest_symlink;

  /* This is a set of destination name/inode/dev triples.  Each such triple
     represents a file we have created corresponding to a source file name
     that was specified on the command line.  Use it to avoid clobbering
     source files in commands like this:
       rm -rf a b c; mkdir a b c; touch a/f b/f; mv a/f b/f c
     For now, it protects only regular files when copying (i.e. not renaming).
     When renaming, it protects all non-directories.
     Use dest_info_init to initialize it, or set it to NULL to disable
     this feature.  */
  Hash_table *dest_info;

  /* FIXME */
  Hash_table *src_info;
};

#define	STDIN_FILENO	0	/* Standard input.  */

#define	ENOENT		 2	/* No such file or directory */



# ifndef DIRECTORY_SEPARATOR
#  define DIRECTORY_SEPARATOR '/'
# endif

# ifndef ISSLASH
#  define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)
# endif




int errno;
// ----------------------------------------------------------------


/* Print the program name and error message MESSAGE, which is a printf-style
   format string with optional args.
   If ERRNUM is nonzero, print its corresponding system error message.
   Exit with status STATUS if it is nonzero.  */
void
error (int status, int errnum, const char *message, ...)
{
	if(errnum!=0) exit(1);
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
		return get_first_opt(optarg); /* RETURN POINT 3 */
	}
	return -1; /* RETURN POINT 4 */
}




bool lstat(char* c, struct stat * b)
{
	b->st_ino = unknown();
	b->st_dev = unknown();
	if(unknown()) return false;
	return true;
}

/* Call lstat to get the device and inode numbers for `/'.
   Upon failure, return NULL.  Otherwise, set the members of
   *ROOT_D_I accordingly and return ROOT_D_I.  */
struct dev_ino *
get_root_dev_ino (struct dev_ino *root_d_i)
{
  struct stat statbuf;
  if (lstat ("/", &statbuf))
    return NULL;
   static_assert(root_d_i != NULL);
  root_d_i->st_ino = statbuf.st_ino;
  root_d_i->st_dev = statbuf.st_dev;
  return root_d_i;
}


static void
rm_option_init (struct rm_options *x)
{
  static_assert(x!=NULL);
  x->ignore_missing_files = false;
  x->recursive = true;
  x->one_file_system = false;

  /* Should we prompt for removal, too?  No.  Prompting for the `move'
     part is enough.  It implies removal.  */
  x->interactive = RMI_NEVER;
  x->stdin_tty = false;

  x->verbose = false;

  /* Since this program may well have to process additional command
     line arguments after any call to `rm', that function must preserve
     the initial working directory, in case one of those is a
     `.'-relative name.  */
  x->require_restore_cwd = true;

  {
    static struct dev_ino dev_ino_buf;
    x->root_dev_ino = get_root_dev_ino (&dev_ino_buf);
    if (x->root_dev_ino == NULL)
      error (EXIT_FAILURE, errno, _("failed to get attributes of %s"),
	     quote ("/")); 
  }
}

/* Set *X to the default options for a value of type struct cp_options.  */

extern void
cp_options_default (struct cp_options *x)
{
 static_assert(x!=NULL);
  memset (x, 0, sizeof *x);
#ifdef PRIV_FILE_CHOWN
  {
    priv_set_t *pset = priv_allocset ();
    if (!pset)
      xalloc_die ();
    if (getppriv (PRIV_EFFECTIVE, pset) == 0)
      {
	x->chown_privileges = priv_ismember (pset, PRIV_FILE_CHOWN);
	x->owner_privileges = priv_ismember (pset, PRIV_FILE_OWNER);
      }
    priv_freeset (pset);
  }
#else
  x->chown_privileges = x->owner_privileges = (geteuid () == 0);
#endif
}


static void
cp_option_init (struct cp_options *x)
{
  static_assert(x!=NULL);
  bool selinux_enabled = (0 < is_selinux_enabled ());

  cp_options_default (x);
  x->copy_as_regular = false;  
  x->dereference = DEREF_NEVER;
  x->unlink_dest_before_opening = false;
  x->unlink_dest_after_failed_open = false;
  x->hard_link = false;
  x->interactive = I_UNSPECIFIED;
  x->move_mode = true;
  x->one_file_system = false;
  x->preserve_ownership = true;
  x->preserve_links = true;
  x->preserve_mode = true;
  x->preserve_timestamps = true;
  x->preserve_security_context = selinux_enabled;
  x->reduce_diagnostics = false;
  x->require_preserve = false;  
  x->require_preserve_context = false;
  x->preserve_xattr = true;
  x->require_preserve_xattr = false;
  x->recursive = true;
  x->sparse_mode = SPARSE_AUTO; 
  x->symbolic_link = false;
  x->set_mode = false;
  x->mode = 0;
  x->stdin_tty = isatty (STDIN_FILENO);

  x->open_dangling_dest_symlink = false;
  x->update = false;
  x->verbose = false;
  x->dest_info = NULL;
  x->src_info = NULL;
}
/* FILE is the last operand of this command.  Return true if FILE is a
   directory.  But report an error if there is a problem accessing FILE, other
   than nonexistence (errno == ENOENT).  */

static bool
target_directory_operand (char const *file)
{
  struct stat st;
  int err = (stat (file, &st) == 0 ? 0 : errno);
  bool is_a_dir = !err && S_ISDIR (st.st_mode);
  if (err && err != ENOENT)
    error (EXIT_FAILURE, err, _("accessing %s"), quote (file));
  return is_a_dir;
}

bool
copy (char const *src_name, char const *dst_name,
      bool nonexistent_dst, const struct cp_options *options,
      bool *copy_into_self, bool *rename_succeeded)
{
	if(unknown()) return false;
	return true;
}


static bool
do_move (const char *source, const char *dest, const struct cp_options *x)
{
  bool copy_into_self;
  bool rename_succeeded;
  bool ok = copy (source, dest, false, x, &copy_into_self, &rename_succeeded);

  if (ok)
    {
      char const *dir_to_remove;
      if (copy_into_self)
	{
	  /* In general, when copy returns with copy_into_self set, SOURCE is
	     the same as, or a parent of DEST.  In this case we know it's a
	     parent.  It doesn't make sense to move a directory into itself, and
	     besides in some situations doing so would give highly nonintuitive
	     results.  Run this `mkdir b; touch a c; mv * b' in an empty
	     directory.  Here's the result of running echo `find b -print`:
	     b b/a b/b b/b/a b/c.  Notice that only file `a' was copied
	     into b/b.  Handle this by giving a diagnostic, removing the
	     copied-into-self directory, DEST (`b/b' in the example),
	     and failing.  */

	  dir_to_remove = NULL;
	  ok = false;
	}
      else if (rename_succeeded)
	{
	  /* No need to remove anything.  SOURCE was successfully
	     renamed to DEST.  Or the user declined to rename a file.  */
	  dir_to_remove = NULL;
	}
      else
	{
	  /* This may mean SOURCE and DEST referred to different devices.
	     It may also conceivably mean that even though they referred
	     to the same device, rename wasn't implemented for that device.

	     E.g., (from Joel N. Weber),
	     [...] there might someday be cases where you can't rename
	     but you can copy where the device name is the same, especially
	     on Hurd.  Consider an ftpfs with a primitive ftp server that
	     supports uploading, downloading and deleting, but not renaming.

	     Also, note that comparing device numbers is not a reliable
	     check for `can-rename'.  Some systems can be set up so that
	     files from many different physical devices all have the same
	     st_dev field.  This is a feature of some NFS mounting
	     configurations.

	     We reach this point if SOURCE has been successfully copied
	     to DEST.  Now we have to remove SOURCE.

	     This function used to resort to copying only when rename
	     failed and set errno to EXDEV.  */

	  dir_to_remove = source;
	}

      if (dir_to_remove != NULL)
	{
	  struct rm_options rm_options;
	  enum RM_status status;

	  rm_option_init (&rm_options);
	  static_assert(x!=NULL);
	  rm_options.verbose = x->verbose;

	  status = rm (1, &dir_to_remove, &rm_options);
	  assert (VALID_STATUS (status));
	  if (status == RM_ERROR)
	    ok = false;
	}
    }

  return ok;
}




/* Move file SOURCE onto DEST.  Handles the case when DEST is a directory.
   Treat DEST as a directory if DEST_IS_DIR.
   Return true if successful.  */

static bool
movefile (char *source, char *dest, bool dest_is_dir,
	  const struct cp_options *x)
{
  bool ok;

  /* This code was introduced to handle the ambiguity in the semantics
     of mv that is induced by the varying semantics of the rename function.
     Some systems (e.g., Linux) have a rename function that honors a
     trailing slash, while others (like Solaris 5,6,7) have a rename
     function that ignores a trailing slash.  I believe the Linux
     rename semantics are POSIX and susv2 compliant.  */

  if (remove_trailing_slashes)
    strip_trailing_slashes (source);

  if (dest_is_dir)
    {
      /* Treat DEST as a directory; build the full filename.  */
      char const *src_basename = last_component (source);
      char *new_dest = file_name_concat (dest, src_basename, NULL);
      strip_trailing_slashes (new_dest);
      ok = do_move (source, new_dest, x);
      free (new_dest);
    }
  else
    {
      ok = do_move (source, dest, x);
    }

  return ok;
}


void
usage (int status)
{

  exit (status);
}


int
main (int argc, char **argv)
{
  assume(argv != NULL);
  assume_size(argv, argc);

  int c;
  bool ok;
  bool make_backups = false;
  char *backup_suffix_string;
  char *version_control_string = NULL;
  struct cp_options x;
  char *target_directory = NULL;
  bool no_target_directory = false;
  int n_files;
  char **file;

  initialize_main (&argc, &argv);
  set_program_name (argv[0]);
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  cp_option_init (&x);
 
  // FIXME: consider not calling getenv for SIMPLE_BACKUP_SUFFIX unless
   //  we'll actually use backup_suffix_string.  
  backup_suffix_string = getenv ("SIMPLE_BACKUP_SUFFIX");

  while ((c = getopt_long (argc, argv,"bfint:uvS:T", NULL, NULL))
	 != -1)
    {
    
      switch (c)
	{
	case 'b':
	  make_backups = true;
	//  if (optarg)
	  //  version_control_string = optarg;
	  break; 
	case 'f':
	  x.interactive = I_ALWAYS_YES;
	  break;
	case 'i':
	  x.interactive = I_ASK_USER;
	  break;
	case 'n':
	  x.interactive = I_ALWAYS_NO;
	  break;
	case STRIP_TRAILING_SLASHES_OPTION:
	  remove_trailing_slashes = true;
	  break;
	case 't':
	  if (target_directory)
	    error (EXIT_FAILURE, 0, _("multiple target directories specified"));
	  else
	    {
	      struct stat st;
	      if (stat (optarg, &st) != 0)
		error (EXIT_FAILURE, errno, _("accessing %s"), "");
	      if (! S_ISDIR (st.st_mode))
		error (EXIT_FAILURE, 0, _("target %s is not a directory"),
		       "");
	    }
	//  target_directory = optarg;
	  break;
	case 'T':
	  no_target_directory = true;
	  break;
	case 'u':
	  x.update = true;
	  break;
	case 'v':
	  x.verbose = true;
	  break;
	case 'S':
	  make_backups = true;
	 // backup_suffix_string = optarg;
	  break;
	case_GETOPT_HELP_CHAR;
	case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);
	default:
	  usage (EXIT_FAILURE);
	}
	
    }

    

  n_files = argc - optind;
  file = argv + optind;


  if (n_files <= !target_directory)
    {
      if (n_files <= 0)
	error (0, 0, _("missing file operand"));
      else {
      	buffer_safe(file, 0);
	error (0, 0, _("missing destination file operand after %s"),
	       quote (file[0]));
	 }
      usage (EXIT_FAILURE);
    }

  if (no_target_directory)
    {
      if (target_directory)
	error (EXIT_FAILURE, 0,
	       _("cannot combine --target-directory (-t) "
		 "and --no-target-directory (-T)"));
      if (2 < n_files)
	{
	  buffer_safe(file, 2);
	  error (0, 0, _("extra operand %s"), quote (file[2]));
	  usage (EXIT_FAILURE);
	}
    }
  else if (!target_directory)
    {
      assert (2 <= n_files);
      buffer_safe(file, n_files-1);
      if (target_directory_operand (file[n_files - 1]))
	target_directory = file[--n_files];
      else if (2 < n_files)
	error (EXIT_FAILURE, 0, _("target %s is not a directory"),
	       quote (file[n_files - 1]));
    }

  if (make_backups && x.interactive == I_ALWAYS_NO)
    {
      error (0, 0,
	     _("options --backup and --no-clobber are mutually exclusive"));
      usage (EXIT_FAILURE);
    }

  if (backup_suffix_string)
    simple_backup_suffix = xstrdup (backup_suffix_string);

  x.backup_type = (make_backups
		   ? xget_version (_("backup type"),
				   version_control_string)
		   : no_backups);
		   
 

  hash_init ();

  if (target_directory)
    {
      int i;

      // Initialize the hash table only if we'll need it.
	// The problem it is used to detect can arise only if there are
	 //two or more files to move.  
      if (2 <= n_files)
	dest_info_init (&x);

      ok = true;
      for (i = 0; i < n_files; ++i) {
      	buffer_safe(file, i);
	ok &= movefile (file[i], target_directory, true, &x);
	}
    }
  else {
    buffer_safe(file, 1);
    ok = movefile (file[0], file[1], false, &x);
   }

  exit (ok ? EXIT_SUCCESS : EXIT_FAILURE);
  
  
}
