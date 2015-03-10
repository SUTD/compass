//SSH/sshconnect.c/timeout_connect

void specify_checks()
{
	check_null();
	check_buffer();
}

#include <stdlib.h>

#define FD_SET ignore

#  define NI_MAXHOST      1025
#  define NI_MAXSERV      32
# define _PATH_BSHELL "/bin/sh"
#define SIZE_T_MAX 10000



typedef unsigned char u_char;
typedef unsigned short int u_short;
typedef unsigned int u_int;
typedef unsigned long int u_long;
typedef int pid_t;
typedef unsigned int socklen_t;

#define	EINPROGRESS	115	/* Operation now in progress */
#define	ETIMEDOUT	110	/* Connection timed out */
#define	EINTR		 4	/* Interrupted system call */

#define SOL_SOCKET	1
#define SO_ERROR	4

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

//----------------------------------------

/* Fatal messages.  This function never returns. */

void
fatal(const char *fmt,...)
{
	exit(1);
}



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


void
ms_to_timeval(struct timeval *tv, int ms)
{
	if (ms < 0)
		ms = 0;
	tv->tv_sec = ms / 1000;
	tv->tv_usec = (ms % 1000) * 1000;
}

void
ms_subtract_diff(struct timeval *start, int *ms)
{
	struct timeval diff, finish;

	gettimeofday(&finish, NULL);
	timersub(&finish, start, &diff);	
	*ms -= (diff.tv_sec * 1000) + (diff.tv_usec / 1000);
}


static int
timeout_connect(int sockfd, const struct sockaddr *serv_addr,
    socklen_t addrlen, int *timeoutp)
{
	assume(timeoutp != NULL);

	fd_set *fdset;
	struct timeval tv, t_start;
	socklen_t optlen;
	int optval, rc, result = -1;

	gettimeofday(&t_start, NULL);

	if (*timeoutp <= 0) {
		result = connect(sockfd, serv_addr, addrlen);
		goto done;
	}

	set_nonblock(sockfd);
	rc = connect(sockfd, serv_addr, addrlen);
	if (rc == 0) {
		unset_nonblock(sockfd);
		result = 0;
		goto done;
	}
	if (errno != EINPROGRESS) {
		result = -1;
		goto done;
	}

	fdset = (fd_set *)xcalloc(howmany(sockfd + 1, NFDBITS),
	    sizeof(fd_mask));
	FD_SET(sockfd, fdset);
	ms_to_timeval(&tv, *timeoutp);

	for (;;) {
		rc = select(sockfd + 1, NULL, fdset, NULL, &tv);
		if (rc != -1 || errno != EINTR)
			break;
	}

	switch (rc) {
	case 0:
		/* Timed out */
		errno = ETIMEDOUT;
		break;
	case -1:
		/* Select error */
		debug("select: %s", strerror(errno));
		break;
	case 1:
		/* Completed or failed */
		optval = 0;
		optlen = sizeof(optval);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval,
		    &optlen) == -1) {
			debug("getsockopt: %s", strerror(errno));
			break;
		}
		if (optval != 0) {
			errno = optval;
			break;
		}
		result = 0;
		unset_nonblock(sockfd);
		break;
	default:
		/* Should not occur */
		fatal("Bogus return (%d) from select()", rc);
	}

	xfree(fdset);

 done:
 	if (result == 0 && *timeoutp > 0) {
		ms_subtract_diff(&t_start, timeoutp);
		if (*timeoutp <= 0) {
			errno = ETIMEDOUT;
			result = -1;
		}
	}

	return (result);
}