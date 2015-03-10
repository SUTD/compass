//SSH/sshconnect.c/confirm

void specify_checks()
{
	check_null();
	check_buffer();
}

#include <stdlib.h>

#  define NI_MAXHOST      1025
#  define NI_MAXSERV      32
# define _PATH_BSHELL "/bin/sh"
#define SIZE_T_MAX 10000

#define SSH_VERSION	"OpenSSH_5.3"

#define RP_ECHO			0x0001
#define RP_ALLOW_STDIN		0x0002
#define RP_ALLOW_EOF		0x0004
#define RP_USE_ASKPASS		0x0008

#define	EINVAL		22	/* Invalid argument */
#define	ENOTTY		25	/* Not a typewriter */


/*
 * Major protocol version.  Different version indicates major incompatibility
 * that prevents communication.
 *
 * Minor protocol version.  Different version indicates minor incompatibility
 * that does not prevent interoperation.
 */
#define PROTOCOL_MAJOR_1	1
#define PROTOCOL_MINOR_1	5

/* We support both SSH1 and SSH2 */
#define PROTOCOL_MAJOR_2	2
#define PROTOCOL_MINOR_2	0

#define	SSH_PROTO_UNKNOWN	0x00
#define	SSH_PROTO_1		0x01
#define	SSH_PROTO_1_PREFERRED	0x02
#define	SSH_PROTO_2		0x04

#define SSH_BUG_SIGBLOB		0x00000001
#define SSH_BUG_PKSERVICE	0x00000002
#define SSH_BUG_HMAC		0x00000004
#define SSH_BUG_X11FWD		0x00000008
#define SSH_OLD_SESSIONID	0x00000010
#define SSH_BUG_PKAUTH		0x00000020
#define SSH_BUG_DEBUG		0x00000040
#define SSH_BUG_BANNER		0x00000080
#define SSH_BUG_IGNOREMSG	0x00000100
#define SSH_BUG_PKOK		0x00000200
#define SSH_BUG_PASSWORDPAD	0x00000400
#define SSH_BUG_SCANNER		0x00000800
#define SSH_BUG_BIGENDIANAES	0x00001000
#define SSH_BUG_RSASIGMD5	0x00002000
#define SSH_OLD_DHGEX		0x00004000
#define SSH_BUG_NOREKEY		0x00008000
#define SSH_BUG_HBSERVICE	0x00010000
#define SSH_BUG_OPENFAILURE	0x00020000
#define SSH_BUG_DERIVEKEY	0x00040000
#define SSH_BUG_DUMMYCHAN	0x00100000
#define SSH_BUG_EXTEOF		0x00200000
#define SSH_BUG_PROBE		0x00400000
#define SSH_BUG_FIRSTKEX	0x00800000
#define SSH_OLD_FORWARD_ADDR	0x01000000
#define SSH_BUG_RFWD_ADDR	0x02000000
#define SSH_NEW_OPENSSH		0x04000000



typedef unsigned char u_char;
typedef unsigned short int u_short;
typedef unsigned int u_int;
typedef unsigned long int u_long;
typedef int pid_t;
typedef unsigned int socklen_t;

/* Read NBYTES into BUF from FD.  Return the
   number read, -1 for errors or 0 for EOF.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t read (int __fd, void *__buf, size_t __nbytes) __wur;

#define	EINPROGRESS	115	/* Operation now in progress */
#define	ETIMEDOUT	110	/* Connection timed out */
#define	EINTR		 4	/* Interrupted system call */

#define SOL_SOCKET	1
#define SO_ERROR	4
#define SO_KEEPALIVE	9

#define RPP_ECHO_OFF    0x00		/* Turn off echo (default). */
#define RPP_ECHO_ON     0x01		/* Leave echo on. */
#define RPP_REQUIRE_TTY 0x02		/* Fail if there is no tty. */
#define RPP_FORCELOWER  0x04		/* Force input to lower case. */
#define RPP_FORCEUPPER  0x08		/* Force input to upper case. */
#define RPP_SEVENBIT    0x10		/* Strip the high bit from input. */
#define RPP_STDIN       0x20		/* Read from stdin, not /dev/tty */

/* open/fcntl - O_SYNC is only implemented on blocks devices and on files
   located on an ext2 file system */
#define O_ACCMODE	   0003
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#define O_CREAT		   0100	/* not fcntl */
#define O_EXCL		   0200	/* not fcntl */
#define O_NOCTTY	   0400	/* not fcntl */
#define O_TRUNC		  01000	/* not fcntl */
#define O_APPEND	  02000
#define O_NONBLOCK	  04000
#define O_NDELAY	O_NONBLOCK
#define O_SYNC		 010000
#define O_FSYNC		 O_SYNC
#define O_ASYNC		 020000

# define _PATH_TTY "/dev/tty"
#define _PATH_SSH_ASKPASS_DEFAULT	"/usr/X11R6/bin/ssh-askpass"

/* Standard file descriptors.  */
#define	STDIN_FILENO	0	/* Standard input.  */
#define	STDOUT_FILENO	1	/* Standard output.  */
#define	STDERR_FILENO	2	/* Standard error output.  */
/*
 * Maximum number of RSA authentication identity files that can be specified
 * in configuration files or on the command line.
 */
#define SSH_MAX_IDENTITY_FILES		100

/* Maximum number of TCP/IP ports forwarded per direction. */
#define SSH_MAX_FORWARDS_PER_DIRECTION	100

#define MAX_SEND_ENV	256

/* Highest reserved Internet port number.  */
# define IPPORT_RESERVED	1024

/* Possible values for `ai_flags' field in `addrinfo' structure.  */
# define AI_PASSIVE	0x0001	/* Socket address is intended for `bind'.  */
# define AI_CANONNAME	0x0002	/* Request for canonical name.  */
# define AI_NUMERICHOST	0x0004	/* Don't use name resolution.  */
# define AI_V4MAPPED	0x0008	/* IPv4 mapped addresses are acceptable.  */
# define AI_ALL		0x0010	/* Return IPv4 mapped and IPv6 addresses.  */
# define AI_ADDRCONFIG	0x0020	/* Use configuration of this host to choose
				   returned address type..  */
				   
#define AF_INET 2
#define AF_INET6 10

# define NI_NUMERICHOST	1	/* Don't try to look up hostname.  */
# define NI_NUMERICSERV 2	/* Don't convert port number to name.  */
# define NI_NOFQDN	4	/* Only return nodename portion.  */
# define NI_NAMEREQD	8	/* Don't return numeric addresses.  */
# define NI_DGRAM	16	/* Look up UDP service rather than TCP.  */

# define howmany(x, y)	(((x) + ((y) - 1)) / (y))

#define	EPIPE		32	/* Broken pipe */

/*
 * Environment variable for overwriting the default location of askpass
 */
#define SSH_ASKPASS_ENV		"SSH_ASKPASS"

/* readpass.c */

#define RP_ECHO			0x0001
#define RP_ALLOW_STDIN		0x0002
#define RP_ALLOW_EOF		0x0004
#define RP_USE_ASKPASS		0x0008

typedef enum {
	SYSLOG_LEVEL_QUIET,
	SYSLOG_LEVEL_FATAL,
	SYSLOG_LEVEL_ERROR,
	SYSLOG_LEVEL_INFO,
	SYSLOG_LEVEL_VERBOSE,
	SYSLOG_LEVEL_DEBUG1,
	SYSLOG_LEVEL_DEBUG2,
	SYSLOG_LEVEL_DEBUG3,
	SYSLOG_LEVEL_NOT_SET = -1
}       LogLevel;

/* Supported syslog facilities and levels. */
typedef enum {
	SYSLOG_FACILITY_DAEMON,
	SYSLOG_FACILITY_USER,
	SYSLOG_FACILITY_AUTH,
#ifdef LOG_AUTHPRIV
	SYSLOG_FACILITY_AUTHPRIV,
#endif
	SYSLOG_FACILITY_LOCAL0,
	SYSLOG_FACILITY_LOCAL1,
	SYSLOG_FACILITY_LOCAL2,
	SYSLOG_FACILITY_LOCAL3,
	SYSLOG_FACILITY_LOCAL4,
	SYSLOG_FACILITY_LOCAL5,
	SYSLOG_FACILITY_LOCAL6,
	SYSLOG_FACILITY_LOCAL7,
	SYSLOG_FACILITY_NOT_SET = -1
}       SyslogFacility;

typedef struct Key Key;
enum types {
	KEY_RSA1,
	KEY_RSA,
	KEY_DSA,
	KEY_UNSPEC
};
enum fp_type {
	SSH_FP_SHA1,
	SSH_FP_MD5
};
enum fp_rep {
	SSH_FP_HEX,
	SSH_FP_BUBBLEBABBLE,
	SSH_FP_RANDOMART
};

/* key is stored in external hardware */
#define KEY_FLAG_EXT		0x0001

struct Key {
	int	 type;
	int	 flags;
	int	*rsa;
	int	*dsa;
};



/* Data structure for representing a forwarding request. */

typedef struct {
	char	 *listen_host;		/* Host (address) to listen on. */
	int	  listen_port;		/* Port to forward. */
	char	 *connect_host;		/* Host to connect. */
	int	  connect_port;		/* Port to connect on connect_host. */
}       Forward;

typedef struct {
	int     forward_agent;	/* Forward authentication agent. */
	int     forward_x11;	/* Forward X11 display. */
	int     forward_x11_trusted;	/* Trust Forward X11 display. */
	int     exit_on_forward_failure;	/* Exit if bind(2) fails for
-L/-R */
	char   *xauth_location;	/* Location for xauth program */
	int     gateway_ports;	/* Allow remote connects to forwarded ports. */
	int     use_privileged_port;	/* Don't use privileged port if false.
*/
	int     rhosts_rsa_authentication;	/* Try rhosts with RSA
						 * authentication. */
	int     rsa_authentication;	/* Try RSA authentication. */
	int     pubkey_authentication;	/* Try ssh2 pubkey authentication. */
	int     hostbased_authentication;	/* ssh2's rhosts_rsa */
	int     challenge_response_authentication;
					/* Try S/Key or TIS, authentication. */
	int     gss_authentication;	/* Try GSS authentication */
	int     gss_deleg_creds;	/* Delegate GSS credentials */
	int     password_authentication;	/* Try password
						 * authentication. */
	int     kbd_interactive_authentication; /* Try keyboard-interactive
auth. */
	char	*kbd_interactive_devices; /* Keyboard-interactive auth devices.
*/
	int     zero_knowledge_password_authentication;	/* Try jpake */
	int     batch_mode;	/* Batch mode: do not ask for passwords. */
	int     check_host_ip;	/* Also keep track of keys for IP address */
	int     strict_host_key_checking;	/* Strict host key checking. */
	int     compression;	/* Compress packets in both directions. */
	int     compression_level;	/* Compression level 1 (fast) to 9
					 * (best). */
	int     tcp_keep_alive;	/* Set SO_KEEPALIVE. */
	LogLevel log_level;	/* Level for logging. */

	int     port;		/* Port to connect. */
	int     address_family;
	int     connection_attempts;	/* Max attempts (seconds) before
					 * giving up */
	int     connection_timeout;	/* Max time (seconds) before
					 * aborting connection attempt */
	int     number_of_password_prompts;	/* Max number of password
						 * prompts. */
	int     cipher;		/* Cipher to use. */
	char   *ciphers;	/* SSH2 ciphers in order of preference. */
	char   *macs;		/* SSH2 macs in order of preference. */
	char   *hostkeyalgorithms;	/* SSH2 server key types in order of
preference. */
	int	protocol;	/* Protocol in order of preference. */
	char   *hostname;	/* Real host to connect. */
	char   *host_key_alias;	/* hostname alias for .ssh/known_hosts */
	char   *proxy_command;	/* Proxy command for connecting the host. */
	char   *user;		/* User to log in as. */
	int     escape_char;	/* Escape character; -2 = none */

	char   *system_hostfile;/* Path for /etc/ssh/ssh_known_hosts. */
	char   *user_hostfile;	/* Path for $HOME/.ssh/known_hosts. */
	char   *system_hostfile2;
	char   *user_hostfile2;
	char   *preferred_authentications;
	char   *bind_address;	/* local socket address for connection to sshd
*/
	char   *smartcard_device; /* Smartcard reader device */
	int	verify_host_key_dns;	/* Verify host key using DNS */

	int     num_identity_files;	/* Number of files for RSA/DSA
identities. */
	char   *identity_files[SSH_MAX_IDENTITY_FILES];
	Key    *identity_keys[SSH_MAX_IDENTITY_FILES];

	/* Local TCP/IP forward requests. */
	int     num_local_forwards;
	Forward local_forwards[SSH_MAX_FORWARDS_PER_DIRECTION];

	/* Remote TCP/IP forward requests. */
	int     num_remote_forwards;
	Forward remote_forwards[SSH_MAX_FORWARDS_PER_DIRECTION];
	int	clear_forwardings;

	int	enable_ssh_keysign;
	int64_t rekey_limit;
	int	no_host_authentication_for_localhost;
	int	identities_only;
	int	server_alive_interval;
	int	server_alive_count_max;

	int     num_send_env;
	char   *send_env[MAX_SEND_ENV];

	char	*control_path;
	int	control_master;

	int	hash_known_hosts;

	int	tun_open;	/* tun(4) */
	int     tun_local;	/* force tun device (optional) */
	int     tun_remote;	/* force tun device (optional) */

	char	*local_command;
	int	permit_local_command;
	int	visual_host_key;

	int	use_roaming;

}       Options;

struct addrinfo {
	int	ai_flags;	/* AI_PASSIVE, AI_CANONNAME */
	int	ai_family;	/* PF_xxx */
	int	ai_socktype;	/* SOCK_xxx */
	int	ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
	size_t	ai_addrlen;	/* length of ai_addr */
	char	*ai_canonname;	/* canonical name for hostname */
	struct sockaddr *ai_addr;	/* binary address */
	//struct addrinfo *ai_next;	/* next structure in linked list */
};

/* Types of sockets.  */
enum __socket_type
{
  SOCK_STREAM = 1,		/* Sequenced, reliable, connection-based
				   byte streams.  */
#define SOCK_STREAM SOCK_STREAM
  SOCK_DGRAM = 2,		/* Connectionless, unreliable datagrams
				   of fixed maximum length.  */
#define SOCK_DGRAM SOCK_DGRAM
  SOCK_RAW = 3,			/* Raw protocol interface.  */
#define SOCK_RAW SOCK_RAW
  SOCK_RDM = 4,			/* Reliably-delivered messages.  */
#define SOCK_RDM SOCK_RDM
  SOCK_SEQPACKET = 5,		/* Sequenced, reliable, connection-based,
				   datagrams of fixed maximum length.  */
#define SOCK_SEQPACKET SOCK_SEQPACKET
  SOCK_DCCP = 6,		/* Datagram Congestion Control Protocol.  */
#define SOCK_DCCP SOCK_DCCP
  SOCK_PACKET = 10,		/* Linux specific way of getting packets
				   at the dev level.  For writing rarp and
				   other similar things on the user level. */
#define SOCK_PACKET SOCK_PACKET

  /* Flags to be ORed into the type parameter of socket and socketpair and
     used for the flags parameter of paccept.  */

  SOCK_CLOEXEC = 02000000,	/* Atomically set close-on-exec flag for the
				   new descriptor(s).  */
#define SOCK_CLOEXEC SOCK_CLOEXEC
  SOCK_NONBLOCK = 04000		/* Atomically mark descriptor(s) as
				   non-blocking.  */
#define SOCK_NONBLOCK SOCK_NONBLOCK
};
#define ECHO	0000010
#define ECHOE	0000020
#define ECHOK	0000040
#define ECHONL	0000100
#define NOFLSH	0000200
#define TOSTOP	0000400
# define ECHOCTL 0001000
# define ECHOPRT 0002000
# define ECHOKE	 0004000
# define FLUSHO	 0010000
# define PENDIN	 0040000

/* tcsetattr uses these */
#define	TCSANOW		0
#define	TCSADRAIN	1
#define	TCSAFLUSH	2

# define _T_FLUSH	(TCSAFLUSH)

/* Signals.  */
#define	SIGHUP		1	/* Hangup (POSIX).  */
#define	SIGINT		2	/* Interrupt (ANSI).  */
#define	SIGQUIT		3	/* Quit (POSIX).  */
#define	SIGILL		4	/* Illegal instruction (ANSI).  */
#define	SIGTRAP		5	/* Trace trap (POSIX).  */
#define	SIGABRT		6	/* Abort (ANSI).  */
#define	SIGIOT		6	/* IOT trap (4.2 BSD).  */
#define	SIGBUS		7	/* BUS error (4.2 BSD).  */
#define	SIGFPE		8	/* Floating-point exception (ANSI).  */
#define	SIGKILL		9	/* Kill, unblockable (POSIX).  */
#define	SIGUSR1		10	/* User-defined signal 1 (POSIX).  */
#define	SIGSEGV		11	/* Segmentation violation (ANSI).  */
#define	SIGUSR2		12	/* User-defined signal 2 (POSIX).  */
#define	SIGPIPE		13	/* Broken pipe (POSIX).  */
#define	SIGALRM		14	/* Alarm clock (POSIX).  */
#define	SIGTERM		15	/* Termination (ANSI).  */
#define	SIGSTKFLT	16	/* Stack fault.  */
#define	SIGCLD		SIGCHLD	/* Same as SIGCHLD (System V).  */
#define	SIGCHLD		17	/* Child status has changed (POSIX).  */
#define	SIGCONT		18	/* Continue (POSIX).  */
#define	SIGSTOP		19	/* Stop, unblockable (POSIX).  */
#define	SIGTSTP		20	/* Keyboard stop (POSIX).  */
#define	SIGTTIN		21	/* Background read from tty (POSIX).  */
#define	SIGTTOU		22	/* Background write to tty (POSIX).  */
#define	SIGURG		23	/* Urgent condition on socket (4.2 BSD).  */
#define	SIGXCPU		24	/* CPU limit exceeded (4.2 BSD).  */
#define	SIGXFSZ		25	/* File size limit exceeded (4.2 BSD).  */
#define	SIGVTALRM	26	/* Virtual alarm clock (4.2 BSD).  */
#define	SIGPROF		27	/* Profiling alarm clock (4.2 BSD).  */
#define	SIGWINCH	28	/* Window size change (4.3 BSD, Sun).  */
#define	SIGPOLL		SIGIO	/* Pollable event occurred (System V).  */
#define	SIGIO		29	/* I/O now possible (4.2 BSD).  */
#define	SIGPWR		30	/* Power failure restart (System V).  */
#define SIGSYS		31	/* Bad system call.  */
#define SIGUNUSED	31

//----------------------------------------

/* Fatal messages.  This function never returns. */

void
fatal(const char *fmt,...)
{
	exit(1);
}
/*
void * _memset(void *s, int c, size_t n)
{
size_t i;
char * ptr = s;

for (i = 0; i < n; i++, ptr++)
{
	*ptr = c;
}
return s;
}

void * memset(void *s, int c, size_t n)
{
size_t i;
char * ptr = s;

for (i = 0; i < n; i++, ptr++)
{
	*ptr = c;
}
return s;
}


int _getaddrinfo(const char * a, const char *b, 
    const struct addrinfo * c, struct addrinfo ** ai)
{
	*ai = malloc(sizeof(struct addrinfo));
	if(*ai == NULL) return -1;
	return 0;   	
}
*/

// ---------------Callees-------------


int errno;
/* import */
extern Options options;
extern char *__progname;
extern uid_t original_real_uid;
extern uid_t original_effective_uid;
extern pid_t proxy_command_pid;
/*
 * General data structure for command line options and options configurable
 * in configuration files.  See readconf.h.
 */
Options options;

#define getaddrinfo(a,b,c,d)	(ssh_getaddrinfo(a,b,c,d))

/*
 * Macros to raise/lower permissions.
 */
#define PRIV_START do {					\
	int save_errno = errno;				\
	if (seteuid(original_effective_uid) != 0)	\
		fatal("PRIV_START: seteuid: %s",	\
		    strerror(errno));			\
	errno = save_errno;				\
} while (0)

#define PRIV_END do {					\
	int save_errno = errno;				\
	if (seteuid(original_real_uid) != 0)		\
		fatal("PRIV_END: seteuid: %s",		\
		    strerror(errno));			\
	errno = save_errno;				\
} while (0)
void *
xcalloc(size_t nmemb, size_t size)
{
	void *ptr;

	if (size == 0 || nmemb == 0)
		fatal("xcalloc: zero size");
	if (SIZE_T_MAX / nmemb < size)
		fatal("xcalloc: nmemb * size > SIZE_T_MAX");
	ptr = calloc(nmemb, size);
	if (ptr == NULL)
		fatal("xcalloc: out of memory (allocating %lu bytes)",
		    (u_long)(size * nmemb));
	return ptr;
}

void
xfree(void *ptr)
{
	if (ptr == NULL)
		fatal("xfree: NULL pointer given as argument");
	free(ptr);
}

static volatile int signo;

/* Type of a signal handler.  */
typedef void (*__sighandler_t) (int);

/* Structure describing the action to be taken when a signal arrives.  */
struct sigaction
  {
  	/* Used if SA_SIGINFO is not set.  */
	__sighandler_t sa_handler;
  
      /* Additional set of signals to be blocked.  */
    __sigset_t sa_mask;

    /* Special flags.  */
    int sa_flags;

    /* Restore handler.  */
    void (*sa_restorer) (void);
  };

typedef unsigned char	cc_t;
typedef unsigned int	speed_t;
typedef unsigned int	tcflag_t;

#define NCCS 32
struct termios
  {
    tcflag_t c_iflag;		/* input mode flags */
    tcflag_t c_oflag;		/* output mode flags */
    tcflag_t c_cflag;		/* control mode flags */
    tcflag_t c_lflag;		/* local mode flags */
    cc_t c_line;			/* line discipline */
    cc_t c_cc[NCCS];		/* control characters */
    speed_t c_ispeed;		/* input speed */
    speed_t c_ospeed;		/* output speed */
#define _HAVE_STRUCT_TERMIOS_C_ISPEED 1
#define _HAVE_STRUCT_TERMIOS_C_OSPEED 1
  };

static void handler(int s)
{

	signo = s;
}

char *
readpassphrase2(const char *prompt, char *buf, size_t bufsiz, int flags)
{
	ssize_t nr;
	int input, output, save_errno;
	char ch, *p, *end;
	struct termios term, oterm;
	struct sigaction sa, savealrm, saveint, savehup, savequit, saveterm;
	struct sigaction savetstp, savettin, savettou, savepipe;

	/* I suppose we could alloc on demand in this case (XXX). */
	if (bufsiz == 0) {
		errno = EINVAL;
		return(NULL);
	}

restart:
	signo = 0;
	/*
	 * Read and write to /dev/tty if available.  If not, read from
	 * stdin and write to stderr unless a tty is required.
	 */
	if ((flags & RPP_STDIN) ||
	    (input = output = open(_PATH_TTY, O_RDWR)) == -1) {
		if (flags & RPP_REQUIRE_TTY) {
			errno = ENOTTY;
			return(NULL);
		}
		input = STDIN_FILENO;
		output = STDERR_FILENO;
	}

	/*
	 * Catch signals that would otherwise cause the user to end
	 * up with echo turned off in the shell.  Don't worry about
	 * things like SIGXCPU and SIGVTALRM for now.
	 */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;		/* don't restart system calls */
	sa.sa_handler = handler;
	(void)sigaction(SIGALRM, &sa, &savealrm);
	(void)sigaction(SIGHUP, &sa, &savehup);
	(void)sigaction(SIGINT, &sa, &saveint);
	(void)sigaction(SIGPIPE, &sa, &savepipe);
	(void)sigaction(SIGQUIT, &sa, &savequit);
	(void)sigaction(SIGTERM, &sa, &saveterm);
	(void)sigaction(SIGTSTP, &sa, &savetstp);
	(void)sigaction(SIGTTIN, &sa, &savettin);
	(void)sigaction(SIGTTOU, &sa, &savettou);

	/* Turn off echo if possible. */
	if (input != STDIN_FILENO && tcgetattr(input, &oterm) == 0) {
		memcpy(&term, &oterm, sizeof(term));
		if (!(flags & RPP_ECHO_ON))
			term.c_lflag &= ~(ECHO | ECHONL);
#ifdef VSTATUS
		if (term.c_cc[VSTATUS] != _POSIX_VDISABLE)
			term.c_cc[VSTATUS] = _POSIX_VDISABLE;
#endif
		(void)tcsetattr(input, _T_FLUSH, &term);
	} else {
		memset(&term, 0, sizeof(term));
		term.c_lflag |= ECHO;
		memset(&oterm, 0, sizeof(oterm));
		oterm.c_lflag |= ECHO;
	}

	if (!(flags & RPP_STDIN))
		(void)write(output, prompt, strlen(prompt));
	end = buf + bufsiz - 1;
	for (p = buf; p<end; ) {
		nr = read(input, &ch, 1);
		if ((flags & RPP_SEVENBIT))
			ch &= 0x7f;
		if (isalpha(ch)) {
			if ((flags & RPP_FORCELOWER))
				ch = tolower(ch);
			if ((flags & RPP_FORCEUPPER))
				ch = toupper(ch);
		}
		*p++ = ch;
		
	}
	*p = '\0';
	save_errno = errno;
	if (!(term.c_lflag & ECHO))
		(void)write(output, "\n", 1);

	/* Restore old terminal settings and signals. */
	if (memcmp(&term, &oterm, sizeof(term)) != 0) {
		while (tcsetattr(input, _T_FLUSH, &oterm) == -1 &&
		    errno == EINTR)
			continue;
	}
	(void)sigaction(SIGALRM, &savealrm, NULL);
	(void)sigaction(SIGHUP, &savehup, NULL);
	(void)sigaction(SIGINT, &saveint, NULL);
	(void)sigaction(SIGQUIT, &savequit, NULL);
	(void)sigaction(SIGPIPE, &savepipe, NULL);
	(void)sigaction(SIGTERM, &saveterm, NULL);
	(void)sigaction(SIGTSTP, &savetstp, NULL);
	(void)sigaction(SIGTTIN, &savettin, NULL);
	if (input != STDIN_FILENO)
		(void)close(input);

	/*
	 * If we were interrupted by a signal, resend it to ourselves
	 * now that we have restored the signal handlers.
	 */
	if (signo) {
		kill(getpid(), signo);
		switch (signo) {
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			goto restart;
		}
	}

	errno = save_errno;
	return(nr == -1 ? NULL : buf);
}


/*
 * Reads a passphrase from /dev/tty with echo turned off/on.  Returns the
 * passphrase (allocated with xmalloc).  Exits if EOF is encountered. If
 * RP_ALLOW_STDIN is set, the passphrase will be read from stdin if no
 * tty is available
 */
char *
read_passphrase(const char *prompt, int flags)
{
	char *askpass = NULL, *ret, buf[1024];
	int rppflags, use_askpass = 0, ttyfd;

	rppflags = (flags & RP_ECHO) ? RPP_ECHO_ON : RPP_ECHO_OFF;
	if (flags & RP_USE_ASKPASS)
		use_askpass = 1;
	else if (flags & RP_ALLOW_STDIN) {
		if (!isatty(STDIN_FILENO)) {
			debug("read_passphrase: stdin is not a tty");
			use_askpass = 1;
		}
	} else {
		rppflags |= RPP_REQUIRE_TTY;
		ttyfd = open(_PATH_TTY, O_RDWR);
		if (ttyfd >= 0)
			close(ttyfd);
		else {
			debug("read_passphrase: can't open %s: %s", _PATH_TTY,
			    strerror(errno));
			use_askpass = 1;
		}
	}

	if ((flags & RP_USE_ASKPASS) && getenv("DISPLAY") == NULL)
		return (flags & RP_ALLOW_EOF) ? NULL : xstrdup("");

	if (use_askpass && getenv("DISPLAY")) {
		if (getenv(SSH_ASKPASS_ENV))
			askpass = getenv(SSH_ASKPASS_ENV);
		else
			askpass = _PATH_SSH_ASKPASS_DEFAULT;
		if ((ret = ssh_askpass(askpass, prompt)) == NULL)
			if (!(flags & RP_ALLOW_EOF))
				return xstrdup("");
		return ret;
	}

	if (readpassphrase2(prompt, buf, sizeof buf, rppflags) == NULL) {
		if (flags & RP_ALLOW_EOF)
			return NULL;
		return xstrdup("");
	}

	ret = xstrdup(buf);
	memset(buf, 'x', sizeof buf);
	return ret;
}

/* defaults to 'no' */
static int
confirm(const char *prompt)
{
	const char *msg, *again = "Please type 'yes' or 'no': ";
	char *p;
	int ret = -1;

	if (options.batch_mode)
		return 0;
	for (msg = prompt;;msg = again) {
		p = read_passphrase(msg, RP_ECHO);
		if (p == NULL ||
		    (p[0] == '\0') || (p[0] == '\n') ||
		    strncasecmp(p, "no", 2) == 0)
			ret = 0;
		if (p && strncasecmp(p, "yes", 3) == 0)
			ret = 1;
		if (p)
			xfree(p);
		if (ret != -1)
			return ret;
	}
}