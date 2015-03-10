//SSH/sshconnect.c/ssh_connect

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

#define FD_ISSET ignore
#define FD_SET ignore


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
#define SO_KEEPALIVE	9

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

//----------------------------------------

/* Fatal messages.  This function never returns. */

void
fatal(const char *fmt,...)
{
	exit(1);
}

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
permanently_drop_suid(uid_t uid)
{
	uid_t old_uid = getuid();

	debug("permanently_drop_suid: %u", (u_int)uid);
#if defined(HAVE_SETRESUID) && !defined(BROKEN_SETRESUID)
	if (setresuid(uid, uid, uid) < 0)
		fatal("setresuid %u: %.100s", (u_int)uid, strerror(errno));
#elif defined(HAVE_SETREUID) && !defined(BROKEN_SETREUID)
	if (setreuid(uid, uid) < 0)
		fatal("setreuid %u: %.100s", (u_int)uid, strerror(errno));
#else
# ifndef SETEUID_BREAKS_SETUID
	if (seteuid(uid) < 0)
		fatal("seteuid %u: %.100s", (u_int)uid, strerror(errno));
# endif
	if (setuid(uid) < 0)
		fatal("setuid %u: %.100s", (u_int)uid, strerror(errno));
#endif

#ifndef HAVE_CYGWIN
	/* Try restoration of UID if changed (test clearing of saved uid) */
	if (old_uid != uid &&
	    (setuid(old_uid) != -1 || seteuid(old_uid) != -1))
		fatal("%s: was able to restore old [e]uid", __func__);
#endif

	/* Verify UID drop was successful */
	if (getuid() != uid || geteuid() != uid) {
		fatal("%s: euid incorrect uid:%u euid:%u (should be %u)",
		    __func__, (u_int)getuid(), (u_int)geteuid(), (u_int)uid);
	}
}


/*
 * Connect to the given ssh server using a proxy command.
 */
static int
ssh_proxy_connect(const char *host, u_short port, const char *proxy_command)
{
	char *command_string, *tmp;
	int pin[2], pout[2];
	pid_t pid;
	char *shell, strport[NI_MAXSERV];

	if ((shell = getenv("SHELL")) == NULL)
		shell = _PATH_BSHELL;

	/* Convert the port number into a string. */
	snprintf(strport, sizeof strport, "%hu", port);

	/*
	 * Build the final command string in the buffer by making the
	 * appropriate substitutions to the given proxy command.
	 *
	 * Use "exec" to avoid "sh -c" processes on some platforms
	 * (e.g. Solaris)
	 */
	xasprintf(&tmp, "exec %s", proxy_command);
	command_string = percent_expand(tmp, "h", host,
	    "p", strport, (char *)NULL);
	xfree(tmp);

	/* Create pipes for communicating with the proxy. */
	if (pipe(pin) < 0 || pipe(pout) < 0)
		fatal("Could not create pipes to communicate with the proxy:%.100s",
		    strerror(errno));

	debug("Executing proxy command: %.500s", command_string);

	/* Fork and execute the proxy command. */
	if ((pid = fork()) == 0) {
		char *argv[10];

		/* Child.  Permanently give up superuser privileges. */
		permanently_drop_suid(original_real_uid);

		/* Redirect stdin and stdout. */
		close(pin[1]);
		if (pin[0] != 0) {
			if (dup2(pin[0], 0) < 0)
				perror("dup2 stdin");
			close(pin[0]);
		}
		close(pout[0]);
		if (dup2(pout[1], 1) < 0)
			perror("dup2 stdout");
		/* Cannot be 1 because pin allocated two descriptors. */
		close(pout[1]);

		/* Stderr is left as it is so that error messages get
		   printed on the user's terminal. */
		argv[0] = shell;
		argv[1] = "-c";
		argv[2] = command_string;
		argv[3] = NULL;

		/* Execute the proxy command.  Note that we gave up any
		   extra privileges above. */
		execv(argv[0], argv);
		perror(argv[0]);
		exit(1);
	}
	/* Parent. */
	if (pid < 0)
		fatal("fork failed: %.100s", strerror(errno));
	else
		proxy_command_pid = pid; /* save pid to clean up later */

	/* Close child side of the descriptors. */
	close(pin[0]);
	close(pout[1]);

	/* Free the command name. */
	xfree(command_string);

	/* Set the connection file descriptors. */
	packet_set_connection(pout[0], pin[1]);
	packet_set_timeout(options.server_alive_interval,
	    options.server_alive_count_max);

	/* Indicate OK return */
	return 0;
}


/*
 * Creates a (possibly privileged) socket for use as the ssh connection.
 */
static int
ssh_create_socket(int privileged, struct addrinfo *ai)
{

	int sock, gaierr;
	struct addrinfo hints, *res;

	/*
	 * If we are running as root and want to connect to a privileged
	 * port, bind our own socket to a privileged port.
	 */
	if (privileged) {
		int p = IPPORT_RESERVED - 1;
		PRIV_START;
		sock = rresvport_af(&p, ai->ai_family);
		PRIV_END;
		if (sock < 0)
			error("rresvport: af=%d %.100s", ai->ai_family,
			    strerror(errno));
		else
			debug("Allocated local port %d.", p);
		return sock;
	}
	sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (sock < 0)
		error("socket: %.100s", strerror(errno));

	/* Bind the socket to an alternative local IP address */
	if (options.bind_address == NULL)
		return sock;

	_memset(&hints, 0, sizeof(hints));
	hints.ai_family = ai->ai_family;
	hints.ai_socktype = ai->ai_socktype;
	hints.ai_protocol = ai->ai_protocol;
	hints.ai_flags = AI_PASSIVE;
	gaierr = _getaddrinfo(options.bind_address, NULL, &hints, &res);
	if (gaierr) {
		error("getaddrinfo: %s: %s", options.bind_address,
		    ssh_gai_strerror(gaierr));
		close(sock);
		return -1;
	}
	if (bind(sock, res->ai_addr, res->ai_addrlen) < 0) {
		error("bind: %s: %s", options.bind_address, strerror(errno));
		close(sock);
		freeaddrinfo(res);
		return -1;
	}
	freeaddrinfo(res);
	return sock;
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




/*
 * Opens a TCP/IP connection to the remote server on the given host.
 * The address of the remote host will be returned in hostaddr.
 * If port is 0, the default port will be used.  If needpriv is true,
 * a privileged port will be allocated to make the connection.
 * This requires super-user privileges if needpriv is true.
 * Connection_attempts specifies the maximum number of tries (one per
 * second).  If proxy_command is non-NULL, it specifies the command (with %h
 * and %p substituted for host and port, respectively) to use to contact
 * the daemon.
 */
int
ssh_connect(const char *host, struct sockaddr_storage * hostaddr,
    u_short port, int family, int connection_attempts, int *timeout_ms,
    int want_keepalive, int needpriv, const char *proxy_command)
{
	assume(timeout_ms != NULL);

	int gaierr;
	int on = 1;
	int sock = -1, attempt;
	char ntop[NI_MAXHOST], strport[NI_MAXSERV];
	struct addrinfo hints, *ai, *aitop;

	debug2("ssh_connect: needpriv %d", needpriv);
	
	/* If a proxy command is given, connect using it. */
	if (proxy_command != NULL)
		return ssh_proxy_connect(host, port, proxy_command);
		
	/* No proxy command. */

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
	snprintf(strport, sizeof strport, "%u", port);
	if ((gaierr = _getaddrinfo(host, strport, &hints, &aitop)) != 0)
		fatal("%s: Could not resolve hostname %.100s: %s", __progname,
		    host, ssh_gai_strerror(gaierr));
	
	for (attempt = 0; attempt < connection_attempts; attempt++) {
		if (attempt > 0) {
			/* Sleep a moment before retrying. */
			sleep(1);
			debug("Trying again...");
		}
		
		/*
		 * Try if connection succeeds
		 */
		  ai = aitop;
		 if (ai->ai_family == AF_INET || ai->ai_family == AF_INET6)
		 {
			if (getnameinfo(ai->ai_addr, ai->ai_addrlen,
			    ntop, sizeof(ntop), strport, sizeof(strport),
			    NI_NUMERICHOST|NI_NUMERICSERV) != 0) {
				error("ssh_connect: getnameinfo failed");
			}
			
			else {
				debug("Connecting to %.200s [%.100s] port %s.",
				host, ntop, strport);
			}
			
			/* Create a socket for connecting. */
			sock = ssh_create_socket(needpriv, ai);
			
			if (sock >= 0)
			{
				if (timeout_connect(sock, ai->ai_addr, ai->ai_addrlen,
				    timeout_ms) >= 0) {
					/* Successful connection. */
					memcpy(hostaddr, ai->ai_addr, ai->ai_addrlen);
				} else {
					debug("connect to address %s port %s: %s",
					    ntop, strport, strerror(errno));
					close(sock);
					sock = -1;
				}
			}
		 }
		 
		 if (sock != -1)
		break;	/* Successful connection. */

	}
	
		freeaddrinfo(aitop);

	/* Return failure if we didn't get a successful connection. */
	if (sock == -1) {
		error("ssh: connect to host %s port %s: %s",
		    host, strport, strerror(errno));
		return (-1);
	}

	debug("Connection established.");

	/* Set SO_KEEPALIVE if requested. */
	if (want_keepalive &&
	    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&on,
	    sizeof(on)) < 0)
		error("setsockopt SO_KEEPALIVE: %.100s", strerror(errno));

	/* Set the connection. */
	packet_set_connection(sock, sock);
	packet_set_timeout(options.server_alive_interval,
	    options.server_alive_count_max);

	return 0;
}