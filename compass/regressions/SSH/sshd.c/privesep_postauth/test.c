//SSH/sshd.c/privesep_postauth

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


#define MAX_PORTS		256	/* Max # ports. */

#define MAX_ALLOW_USERS		256	/* Max # users on allow list. */
#define MAX_DENY_USERS		256	/* Max # users on deny list. */
#define MAX_ALLOW_GROUPS	256	/* Max # groups on allow list. */
#define MAX_DENY_GROUPS		256	/* Max # groups on deny list. */
#define MAX_SUBSYSTEMS		256	/* Max # subsystems. */
#define MAX_HOSTKEYS		256	/* Max # hostkeys. */
#define MAX_ACCEPT_ENV		256	/* Max # of env vars. */
#define MAX_MATCH_GROUPS	256	/* Max # of groups for Match. */

#define MON_ISAUTH	0x0004	/* Required for Authentication */
#define MON_AUTHDECIDE	0x0008	/* Decides Authentication */
#define MON_ONCE	0x0010	/* Disable after calling */
#define MON_ALOG	0x0020	/* Log auth attempt without authenticating */

#define MON_AUTH	(MON_ISAUTH|MON_AUTHDECIDE)

#define MON_PERMIT	0x1000	/* Request is permitted */

typedef int MD5_CTX;

#define RDRW	0
#define RDONLY	1
#define ROQUIET	2

#define	EINVAL		22	/* Invalid argument */
#define	ENOTTY		25	/* Not a typewriter */

#define SSH_MSG_MIN				1
#define SSH_MSG_MAX				254
/* Message name */			/* msg code */	/* arguments */
#define SSH_MSG_NONE				0	/* no message */
#define SSH_MSG_DISCONNECT			1	/* cause (string) */
#define SSH_SMSG_PUBLIC_KEY			2	/* ck,msk,srvk,hostk */
#define SSH_CMSG_SESSION_KEY			3	/* key (BIGNUM) */
#define SSH_CMSG_USER				4	/* user (string) */
#define SSH_CMSG_AUTH_RHOSTS			5	/* user (string) */
#define SSH_CMSG_AUTH_RSA			6	/* modulus (BIGNUM) */
#define SSH_SMSG_AUTH_RSA_CHALLENGE		7	/* int (BIGNUM) */
#define SSH_CMSG_AUTH_RSA_RESPONSE		8	/* int (BIGNUM) */
#define SSH_CMSG_AUTH_PASSWORD			9	/* pass (string) */
#define SSH_CMSG_REQUEST_PTY			10	/* TERM, tty modes */
#define SSH_CMSG_WINDOW_SIZE			11	/* row,col,xpix,ypix */
#define SSH_CMSG_EXEC_SHELL			12	/* */
#define SSH_CMSG_EXEC_CMD			13	/* cmd (string) */
#define SSH_SMSG_SUCCESS			14	/* */
#define SSH_SMSG_FAILURE			15	/* */
#define SSH_CMSG_STDIN_DATA			16	/* data (string) */
#define SSH_SMSG_STDOUT_DATA			17	/* data (string) */
#define SSH_SMSG_STDERR_DATA			18	/* data (string) */
#define SSH_CMSG_EOF				19	/* */
#define SSH_SMSG_EXITSTATUS			20	/* status (int) */
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION	21	/* channel (int) */
#define SSH_MSG_CHANNEL_OPEN_FAILURE		22	/* channel (int) */
#define SSH_MSG_CHANNEL_DATA			23	/* ch,data (int,str) */
#define SSH_MSG_CHANNEL_CLOSE			24	/* channel (int) */
#define SSH_MSG_CHANNEL_CLOSE_CONFIRMATION	25	/* channel (int) */
/*      SSH_CMSG_X11_REQUEST_FORWARDING		26	   OBSOLETE */
#define SSH_SMSG_X11_OPEN			27	/* channel (int) */
#define SSH_CMSG_PORT_FORWARD_REQUEST		28	/* p,host,hp (i,s,i) */
#define SSH_MSG_PORT_OPEN			29	/* ch,h,p (i,s,i) */
#define SSH_CMSG_AGENT_REQUEST_FORWARDING	30	/* */
#define SSH_SMSG_AGENT_OPEN			31	/* port (int) */
#define SSH_MSG_IGNORE				32	/* string */
#define SSH_CMSG_EXIT_CONFIRMATION		33	/* */
#define SSH_CMSG_X11_REQUEST_FORWARDING		34	/* proto,data (s,s) */
#define SSH_CMSG_AUTH_RHOSTS_RSA		35	/* user,mod (s,mpi) */
#define SSH_MSG_DEBUG				36	/* string */
#define SSH_CMSG_REQUEST_COMPRESSION		37	/* level 1-9 (int) */
#define SSH_CMSG_MAX_PACKET_SIZE		38	/* size 4k-1024k (int) */
#define SSH_CMSG_AUTH_TIS			39	/* we use this for s/key */
#define SSH_SMSG_AUTH_TIS_CHALLENGE		40	/* challenge (string) */
#define SSH_CMSG_AUTH_TIS_RESPONSE		41	/* response (string) */
#define SSH_CMSG_AUTH_KERBEROS			42	/* (KTEXT) */
#define SSH_SMSG_AUTH_KERBEROS_RESPONSE		43	/* (KTEXT) */
#define SSH_CMSG_HAVE_KERBEROS_TGT		44	/* credentials (s) */
#define SSH_CMSG_HAVE_AFS_TOKEN			65	/* token (s) */

/*
 * Authentication methods.  New types can be added, but old types should not
 * be removed for compatibility.  The maximum allowed value is 31.
 */
#define SSH_AUTH_RHOSTS		1
#define SSH_AUTH_RSA		2
#define SSH_AUTH_PASSWORD	3
#define SSH_AUTH_RHOSTS_RSA	4
#define SSH_AUTH_TIS		5
#define SSH_AUTH_KERBEROS	6
#define SSH_PASS_KERBEROS_TGT	7
				/* 8 to 15 are reserved */
#define SSH_PASS_AFS_TOKEN	21

/* Unsigned.  */
typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long int	uint64_t;

#define SSH2_MSG_USERAUTH_REQUEST			50
#define SSH2_MSG_USERAUTH_FAILURE			51
#define SSH2_MSG_USERAUTH_SUCCESS			52
#define SSH2_MSG_USERAUTH_BANNER			53

#define SSH_CMSG_SESSION_KEY			3	/* key (BIGNUM) */
#define SSH_SMSG_SUCCESS			14	/* */

#define DNS_VERIFY_FOUND	0x00000001
#define DNS_VERIFY_MATCH	0x00000002
#define DNS_VERIFY_SECURE	0x00000004

/*
 * Cipher types for SSH-1.  New types can be added, but old types should not
 * be removed for compatibility.  The maximum allowed value is 31.
 */
#define SSH_CIPHER_SSH2		-3
#define SSH_CIPHER_INVALID	-2	/* No valid cipher selected. */
#define SSH_CIPHER_NOT_SET	-1	/* None selected (invalid number). */
#define SSH_CIPHER_NONE		0	/* no encryption */
#define SSH_CIPHER_IDEA		1	/* IDEA CFB */
#define SSH_CIPHER_DES		2	/* DES CBC */
#define SSH_CIPHER_3DES		3	/* 3DES CBC */
#define SSH_CIPHER_BROKEN_TSS	4	/* TRI's Simple Stream encryption CBC */
#define SSH_CIPHER_BROKEN_RC4	5	/* Alleged RC4 */
#define SSH_CIPHER_BLOWFISH	6
#define SSH_CIPHER_RESERVED	7
#define SSH_CIPHER_MAX		31

#define SSH_SMSG_PUBLIC_KEY			2	/* ck,msk,srvk,hostk */

/* Protocol flags.  These are bit masks. */
#define SSH_PROTOFLAG_SCREEN_NUMBER	1	/* X11 forwarding includes screen */
#define SSH_PROTOFLAG_HOST_IN_FWD_OPEN	2	/* forwarding opens contain host */

/*
 * Length of the session key in bytes.  (Specified as 256 bits in the
 * protocol.)
 */
#define SSH_SESSION_KEY_LENGTH		32

/*
 * Force host key length and server key length to differ by at least this
 * many bits.  This is to make double encryption with rsaref work.
 */
#define SSH_KEY_BITS_RESERVED		128


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

#define SSH2_MSG_TRANSPORT_MIN				1
#define SSH2_MSG_TRANSPORT_MAX				49
#define SSH2_MSG_USERAUTH_MIN				50
#define SSH2_MSG_USERAUTH_MAX				79
#define SSH2_MSG_USERAUTH_PER_METHOD_MIN		60
#define SSH2_MSG_USERAUTH_PER_METHOD_MAX		SSH2_MSG_USERAUTH_MAX
#define SSH2_MSG_CONNECTION_MIN				80
#define SSH2_MSG_CONNECTION_MAX				127
#define SSH2_MSG_RESERVED_MIN				128
#define SSH2_MSG_RESERVED_MAX				191
#define SSH2_MSG_LOCAL_MIN				192
#define SSH2_MSG_LOCAL_MAX				255
#define SSH2_MSG_MIN					1
#define SSH2_MSG_MAX					255

#define DNS_RDATACLASS_IN	1
#define DNS_RDATATYPE_SSHFP	44

struct stat
  {
    __dev_t st_dev;		/* Device.  */
#if __WORDSIZE == 32
    unsigned short int __pad1;
#endif
#if __WORDSIZE == 64 || !defined __USE_FILE_OFFSET64
    __ino_t st_ino;		/* File serial number.	*/
#else
    __ino_t __st_ino;			/* 32bit file serial number.	*/
#endif
#if __WORDSIZE == 32
    __mode_t st_mode;			/* File mode.  */
    __nlink_t st_nlink;			/* Link count.  */
#else
    __nlink_t st_nlink;		/* Link count.  */
    __mode_t st_mode;		/* File mode.  */
#endif
    __uid_t st_uid;		/* User ID of the file's owner.	*/
    __gid_t st_gid;		/* Group ID of the file's group.*/
#if __WORDSIZE == 64
    int __pad0;
#endif
    __dev_t st_rdev;		/* Device number, if device.  */
#if __WORDSIZE == 32
    unsigned short int __pad2;
#endif
#if __WORDSIZE == 64 || !defined __USE_FILE_OFFSET64
    __off_t st_size;			/* Size of file, in bytes.  */
#else
    __off64_t st_size;			/* Size of file, in bytes.  */
#endif
    __blksize_t st_blksize;	/* Optimal block size for I/O.  */
#if __WORDSIZE == 64 || !defined __USE_FILE_OFFSET64
    __blkcnt_t st_blocks;		/* Number 512-byte blocks allocated. */
#else
    __blkcnt64_t st_blocks;		/* Number 512-byte blocks allocated. */
#endif
#ifdef __USE_MISC
    /* Nanosecond resolution timestamps are stored in a format
       equivalent to 'struct timespec'.  This is the type used
       whenever possible but the Unix namespace rules do not allow the
       identifier 'timespec' to appear in the <sys/stat.h> header.
       Therefore we have to handle the use of this header in strictly
       standard-compliant sources special.  */
    struct timespec st_atim;		/* Time of last access.  */
    struct timespec st_mtim;		/* Time of last modification.  */
    struct timespec st_ctim;		/* Time of last status change.  */
# define st_atime st_atim.tv_sec	/* Backward compatibility.  */
# define st_mtime st_mtim.tv_sec
# define st_ctime st_ctim.tv_sec
#else
    __time_t st_atime;			/* Time of last access.  */
    unsigned long int st_atimensec;	/* Nscecs of last access.  */
    __time_t st_mtime;			/* Time of last modification.  */
    unsigned long int st_mtimensec;	/* Nsecs of last modification.  */
    __time_t st_ctime;			/* Time of last status change.  */
    unsigned long int st_ctimensec;	/* Nsecs of last status change.  */
#endif
#if __WORDSIZE == 64
    long int __unused[3];
#else
# ifndef __USE_FILE_OFFSET64
    unsigned long int __unused4;
    unsigned long int __unused5;
# else
    __ino64_t st_ino;			/* File serial number.	*/
# endif
#endif
  };
  
  # define RRSET_VALIDATED	1



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

typedef long int BIGNUM;

 struct _RSA
        {
        BIGNUM *n;              // public modulus
        BIGNUM *e;              // public exponent
        BIGNUM *d;              // private exponent
        BIGNUM *p;              // secret prime factor
        BIGNUM *q;              // secret prime factor
        BIGNUM *dmp1;           // d mod (p-1)
        BIGNUM *dmq1;           // d mod (q-1)
        BIGNUM *iqmp;           // q^-1 mod p
        };
        
  struct _DSA
        {
        BIGNUM *p;              // prime number (public)
        BIGNUM *q;              // 160-bit subprime, q | p-1 (public)
        BIGNUM *g;              // generator of subgroup (public)
        BIGNUM *priv_key;       // private key x
        BIGNUM *pub_key;        // public key y = g^x
        };
typedef struct _RSA RSA;
typedef struct _DSA DSA;

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
	RSA	*rsa;
	DSA	*dsa;
};
#define SSH2_MSG_DISCONNECT				1
#define SSH2_MSG_IGNORE					2
#define SSH2_MSG_UNIMPLEMENTED				3
#define SSH2_MSG_DEBUG					4
#define SSH2_MSG_SERVICE_REQUEST			5
#define SSH2_MSG_SERVICE_ACCEPT				6


/* Data structure for representing a forwarding request. */

typedef struct {
	char	 *listen_host;		/* Host (address) to listen on. */
	int	  listen_port;		/* Port to forward. */
	char	 *connect_host;		/* Host to connect. */
	int	  connect_port;		/* Port to connect on connect_host. */
}       Forward;

typedef struct {
	u_int	num_ports;
	u_int	ports_from_cmdline;
	int	ports[MAX_PORTS];	/* Port number to listen on. */
	char   *listen_addr;		/* Address on which the server listens. */
	struct addrinfo *listen_addrs;	/* Addresses on which the server listens. */
	int     address_family;		/* Address family used by the server. */
	char   *host_key_files[MAX_HOSTKEYS];	/* Files containing host keys. */
	int     num_host_key_files;     /* Number of files for host keys. */
	char   *pid_file;	/* Where to put our pid */
	int     server_key_bits;/* Size of the server key. */
	int     login_grace_time;	/* Disconnect if no auth in this time
					 * (sec). */
	int     key_regeneration_time;	/* Server key lifetime (seconds). */
	int     permit_root_login;	/* PERMIT_*, see above */
	int     ignore_rhosts;	/* Ignore .rhosts and .shosts. */
	int     ignore_user_known_hosts;	/* Ignore ~/.ssh/known_hosts
						 * for RhostsRsaAuth */
	int     print_motd;	/* If true, print /etc/motd. */
	int	print_lastlog;	/* If true, print lastlog */
	int     x11_forwarding;	/* If true, permit inet (spoofing) X11 fwd. */
	int     x11_display_offset;	/* What DISPLAY number to start
					 * searching at */
	int     x11_use_localhost;	/* If true, use localhost for fake X11 server. */
	char   *xauth_location;	/* Location of xauth program */
	int     strict_modes;	/* If true, require string home dir modes. */
	int     tcp_keep_alive;	/* If true, set SO_KEEPALIVE. */
	char   *ciphers;	/* Supported SSH2 ciphers. */
	char   *macs;		/* Supported SSH2 macs. */
	int	protocol;	/* Supported protocol versions. */
	int     gateway_ports;	/* If true, allow remote connects to forwarded ports. */
	SyslogFacility log_facility;	/* Facility for system logging. */
	LogLevel log_level;	/* Level for system logging. */
	int     rhosts_rsa_authentication;	/* If true, permit rhosts RSA
						 * authentication. */
	int     hostbased_authentication;	/* If true, permit ssh2 hostbased auth */
	int     hostbased_uses_name_from_packet_only; /* experimental */
	int     rsa_authentication;	/* If true, permit RSA authentication. */
	int     pubkey_authentication;	/* If true, permit ssh2 pubkey authentication. */
	int     kerberos_authentication;	/* If true, permit Kerberos
						 * authentication. */
	int     kerberos_or_local_passwd;	/* If true, permit kerberos
						 * and any other password
						 * authentication mechanism,
						 * such as SecurID or
						 * /etc/passwd */
	int     kerberos_ticket_cleanup;	/* If true, destroy ticket
						 * file on logout. */
	int     kerberos_get_afs_token;		/* If true, try to get AFS token if
						 * authenticated with Kerberos. */
	int     gss_authentication;	/* If true, permit GSSAPI authentication */
	int     gss_cleanup_creds;	/* If true, destroy cred cache on logout */
	int     password_authentication;	/* If true, permit password
						 * authentication. */
	int     kbd_interactive_authentication;	/* If true, permit */
	int     challenge_response_authentication;
	int     zero_knowledge_password_authentication;
					/* If true, permit jpake auth */
	int     permit_empty_passwd;	/* If false, do not permit empty
					 * passwords. */
	int     permit_user_env;	/* If true, read ~/.ssh/environment */
	int     use_login;	/* If true, login(1) is used */
	int     compression;	/* If true, compression is allowed */
	int	allow_tcp_forwarding;
	int	allow_agent_forwarding;
	u_int num_allow_users;
	char   *allow_users[MAX_ALLOW_USERS];
	u_int num_deny_users;
	char   *deny_users[MAX_DENY_USERS];
	u_int num_allow_groups;
	char   *allow_groups[MAX_ALLOW_GROUPS];
	u_int num_deny_groups;
	char   *deny_groups[MAX_DENY_GROUPS];

	u_int num_subsystems;
	char   *subsystem_name[MAX_SUBSYSTEMS];
	char   *subsystem_command[MAX_SUBSYSTEMS];
	char   *subsystem_args[MAX_SUBSYSTEMS];

	u_int num_accept_env;
	char   *accept_env[MAX_ACCEPT_ENV];

	int	max_startups_begin;
	int	max_startups_rate;
	int	max_startups;
	int	max_authtries;
	int	max_sessions;
	char   *banner;			/* SSH-2 banner message */
	int	use_dns;
	int	client_alive_interval;	/*
					 * poke the client this often to
					 * see if it's still there
					 */
	int	client_alive_count_max;	/*
					 * If the client is unresponsive
					 * for this many intervals above,
					 * disconnect the session
					 */

	char   *authorized_keys_file;	/* File containing public keys */
	char   *authorized_keys_file2;

	char   *adm_forced_command;

	int	use_pam;		/* Enable auth via PAM */

	int	permit_tun;

	int	num_permitted_opens;

	char   *chroot_directory;
}       ServerOptions;

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

void *
xmalloc(size_t size)
{
	void *ptr;

	if (size == 0)
		fatal("xmalloc: zero size");
	ptr = malloc(size);
	if (ptr == NULL)
		fatal("xmalloc: out of memory (allocating %lu bytes)", (u_long) size);
	return ptr;
}

char *
xstrdup (s)
  const char *s;
{
  register size_t len = strlen (s) + 1;
  register char *ret = xmalloc (len);
  memcpy (ret, s, len);
  return ret;
}



RSA* RSA_new();
BIGNUM* BN_new();
DSA* DSA_new();
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
extern ServerOptions options;
extern char *__progname;
extern uid_t original_real_uid;
extern uid_t original_effective_uid;
extern pid_t proxy_command_pid;


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

int x;
char *strchr (const char *s, int c)
{
	assume(x>=0 && x<buffer_size(s));
	if(rand()) return &s[x];
	return NULL;
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

typedef struct {
	int family;		/* for example FamilyInternet */
	int length;		/* length of address, in bytes */
	char *address;		/* pointer to where to find the bytes */
} XHostAddress;

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
  
  typedef enum {
	HOST_OK, HOST_NEW, HOST_CHANGED, HOST_FOUND
}       HostStatus;

/* POSIX.1g specifies this type name for the `sa_family' member.  */
typedef unsigned short int sa_family_t;

/* Type to represent a port.  */
typedef unsigned short int in_port_t;

#define	__SOCKADDR_COMMON(sa_prefix) \
  sa_family_t sa_prefix##family

/* Structure describing a generic socket address.  */
struct sockaddr
  {
    __SOCKADDR_COMMON (sa_);	/* Common data: address family and length.  */
    char sa_data[14];		/* Address data.  */
  };
  
  /* Internet address.  */
typedef unsigned int in_addr_t;
struct in_addr
  {
    in_addr_t s_addr;
  };
  #define __SOCKADDR_COMMON_SIZE	(sizeof (unsigned short int))
  
  /* Structure describing an Internet socket address.  */
struct sockaddr_in
  {
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;			/* Port number.  */
    struct in_addr sin_addr;		/* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof (struct sockaddr) -
			   __SOCKADDR_COMMON_SIZE -
			   sizeof (in_port_t) -
			   sizeof (struct in_addr)];
  };

/* Network number for local host loopback.  */
#define	IN_LOOPBACKNET		127



/* IPv6 address */
struct in6_addr
  {
    union
      {
	uint8_t	__u6_addr8[16];
#if defined __USE_MISC || defined __USE_GNU
	uint16_t __u6_addr16[8];
	uint32_t __u6_addr32[4];
#endif
      } __in6_u;
#define s6_addr			__in6_u.__u6_addr8
#if defined __USE_MISC || defined __USE_GNU
# define s6_addr16		__in6_u.__u6_addr16
# define s6_addr32		__in6_u.__u6_addr32
#endif
  };
  
  /* Ditto, for IPv6.  */
struct sockaddr_in6
  {
    __SOCKADDR_COMMON (sin6_);
    in_port_t sin6_port;	/* Transport layer port # */
    uint32_t sin6_flowinfo;	/* IPv6 flow information */
    struct in6_addr sin6_addr;	/* IPv6 address */
    uint32_t sin6_scope_id;	/* IPv6 scope-id */
  };

# define	_SS_MAXSIZE	128	/* Implementation specific max size */
# define       _SS_PADSIZE     (_SS_MAXSIZE - sizeof (struct sockaddr))

struct sockaddr_storage {
	struct sockaddr	ss_sa;
	char		__ss_pad2[_SS_PADSIZE];
};

/* Old OpenSSL doesn't support what we need for DHGEX-sha256 */
#if OPENSSL_VERSION_NUMBER < 0x00907000L
# define KEX_DEFAULT_KEX		\
	"diffie-hellman-group-exchange-sha1," \
	"diffie-hellman-group14-sha1," \
	"diffie-hellman-group1-sha1"
#else
# define KEX_DEFAULT_KEX		\
	"diffie-hellman-group-exchange-sha256," \
	"diffie-hellman-group-exchange-sha1," \
	"diffie-hellman-group14-sha1," \
	"diffie-hellman-group1-sha1"
#endif

#define	KEX_DEFAULT_PK_ALG	"ssh-rsa,ssh-dss"

#define	KEX_DEFAULT_ENCRYPT \
	"aes128-ctr,aes192-ctr,aes256-ctr," \
	"arcfour256,arcfour128," \
	"aes128-cbc,3des-cbc,blowfish-cbc,cast128-cbc," \
	"aes192-cbc,aes256-cbc,arcfour,rijndael-cbc@lysator.liu.se"
#define	KEX_DEFAULT_MAC \
	"hmac-md5,hmac-sha1,umac-64@openssh.com,hmac-ripemd160," \
	"hmac-ripemd160@openssh.com," \
	"hmac-sha1-96,hmac-md5-96"
#define	KEX_DEFAULT_COMP	"none,zlib@openssh.com,zlib"
#define	KEX_DEFAULT_LANG	""

enum kex_init_proposals {
	PROPOSAL_KEX_ALGS,
	PROPOSAL_SERVER_HOST_KEY_ALGS,
	PROPOSAL_ENC_ALGS_CTOS,
	PROPOSAL_ENC_ALGS_STOC,
	PROPOSAL_MAC_ALGS_CTOS,
	PROPOSAL_MAC_ALGS_STOC,
	PROPOSAL_COMP_ALGS_CTOS,
	PROPOSAL_COMP_ALGS_STOC,
	PROPOSAL_LANG_CTOS,
	PROPOSAL_LANG_STOC,
	PROPOSAL_MAX
};

static char *myproposal[PROPOSAL_MAX] = {
	KEX_DEFAULT_KEX,
	KEX_DEFAULT_PK_ALG,
	KEX_DEFAULT_ENCRYPT,
	KEX_DEFAULT_ENCRYPT,
	KEX_DEFAULT_MAC,
	KEX_DEFAULT_MAC,
	KEX_DEFAULT_COMP,
	KEX_DEFAULT_COMP,
	KEX_DEFAULT_LANG,
	KEX_DEFAULT_LANG
};





/* Common definitions for ssh tunnel device forwarding */
#define SSH_TUNMODE_NO		0x00
#define SSH_TUNMODE_POINTOPOINT	0x01
#define SSH_TUNMODE_ETHERNET	0x02
#define SSH_TUNMODE_DEFAULT	SSH_TUNMODE_POINTOPOINT
#define SSH_TUNMODE_YES		(SSH_TUNMODE_POINTOPOINT|SSH_TUNMODE_ETHERNET)

#define SSH_TUNID_ANY		0x7fffffff
#define SSH_TUNID_ERR		(SSH_TUNID_ANY - 1)
#define SSH_TUNID_MAX		(SSH_TUNID_ANY - 2)



char *forced_command;

typedef struct Sensitive Sensitive;
struct Sensitive {
	Key	**keys;
	int	nkeys;
	int	external_keysign;
};

/* The passwd structure.  */
struct passwd
{
  char *pw_name;		/* Username.  */
  char *pw_passwd;		/* Password.  */
  __uid_t pw_uid;		/* User ID.  */
  __gid_t pw_gid;		/* Group ID.  */
  char *pw_gecos;		/* Real name.  */
  char *pw_dir;			/* Home directory.  */
  char *pw_shell;		/* Shell program.  */
};

enum kex_modes {
	MODE_IN,
	MODE_OUT,
	MODE_MAX
};

typedef struct {
	u_char	*buf;		/* Buffer for data. */
	u_int	 alloc;		/* Number of bytes allocated for data. */
	u_int	 offset;	/* Offset of first byte containing data. */
	u_int	 end;		/* Offset of last byte containing data. */
}       Buffer;

typedef int sig_atomic_t;



typedef int EVP_CIPHER;
typedef int EVP_MD;
typedef int HMAC_CTX;
struct Cipher {
	char	*name;
	int	number;		/* for ssh1 only */
	u_int	block_size;
	u_int	key_len;
	u_int	discard_len;
	u_int	cbc_mode;
	const EVP_CIPHER	*(*evptype)(void);
};

typedef struct Cipher Cipher;

struct Enc {
	char	*name;
	Cipher	*cipher;
	int	enabled;
	u_int	key_len;
	u_int	block_size;
	u_char	*key;
	u_char	*iv;
};
typedef struct Enc Enc;
struct Mac {
	char	*name;
	int	enabled;
	u_int	mac_len;
	u_char	*key;
	u_int	key_len;
	int	type;
	const EVP_MD	*evp_md;
	HMAC_CTX	evp_ctx;
	struct umac_ctx *umac_ctx;
};
typedef struct Mac Mac;
struct Comp {
	int	type;
	int	enabled;
	char	*name;
};
typedef struct Comp Comp;

struct Newkeys {
	Enc	enc;
	Mac	mac;
	Comp	comp;
};
typedef struct Newkeys Newkeys;



enum kex_exchange {
	KEX_DH_GRP1_SHA1,
	KEX_DH_GRP14_SHA1,
	KEX_DH_GEX_SHA1,
	KEX_DH_GEX_SHA256,
	KEX_MAX
};



struct Kex;
struct Kex {
	u_char	*session_id;
	u_int	session_id_len;
	Newkeys	*newkeys[MODE_MAX];
	u_int	we_need;
	int	server;
	char	*name;
	int	hostkey_type;
	int	kex_type;
	Buffer	my;
	Buffer	peer;
	sig_atomic_t done;
	int	flags;
	const EVP_MD *evp_md;
	char	*client_version_string;
	char	*server_version_string;
	int	(*verify_host_key)(Key *);
	Key	*(*load_host_key)(int);
	int	(*host_key_index)(Key *);
	void	(*k[KEX_MAX])(int *);
};
typedef struct Kex Kex;

typedef struct {
	int	fd;
	Buffer	identities;
	int	howmany;
}	AuthenticationConnection;

typedef struct Authctxt Authctxt;
typedef struct Authmethod Authmethod;
typedef struct KbdintDevice KbdintDevice;

struct Authctxt {
	sig_atomic_t	 success;
	int		 authenticated;	/* authenticated and alarms cancelled */
	int		 postponed;	/* authentication needs another step */
	int		 valid;		/* user exists and is allowed to login */
	int		 attempt;
	int		 failures;
	int		 force_pwchange;
	char		*user;		/* username sent by the client */
	char		*service;
	struct passwd	*pw;		/* set if 'valid' */
	char		*style;
	void		*kbdintctxt;
	void		*jpake_ctx;
#ifdef BSD_AUTH
	auth_session_t	*as;
#endif
#ifdef KRB5
	krb5_context	 krb5_ctx;
	krb5_ccache	 krb5_fwd_ccache;
	krb5_principal	 krb5_user;
	char		*krb5_ticket_file;
	char		*krb5_ccname;
#endif
	Buffer		*loginmsg;
	void		*methoddata;
};
/*
 * Every authentication method has to handle authentication requests for
 * non-existing users, or for users that are not allowed to login. In this
 * case 'valid' is set to 0, but 'user' points to the username requested by
 * the client.
 */

struct Authmethod {
	char	*name;
	int	(*userauth)(Authctxt *authctxt);
	int	*enabled;
};

/*
 * Keyboard interactive device:
 * init_ctx	returns: non NULL upon success
 * query	returns: 0 - success, otherwise failure
 * respond	returns: 0 - success, 1 - need further interaction,
 *		otherwise - failure
 */
struct KbdintDevice
{
	const char *name;
	void*	(*init_ctx)(Authctxt*);
	int	(*query)(void *ctx, char **name, char **infotxt,
		    u_int *numprompts, char ***prompts, u_int **echo_on);
	int	(*respond)(void *ctx, u_int numresp, char **responses);
	void	(*free_ctx)(void *ctx);
};



/* Write N bytes of BUF to FD.  Return the number written, or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t write (int __fd, __const void *__buf, size_t __n) __wur;
#define vwrite (ssize_t (*)(int, void *, size_t))write

int resume_in_progress;
static u_int64_t write_bytes;
static u_int64_t read_bytes;

size_t
roaming_atomicio(ssize_t(*f)(int, void*, size_t), int fd, void *buf,
    size_t count)
{
	size_t ret = atomicio(f, fd, buf, count);

	if (f == vwrite && ret > 0 && !resume_in_progress) {
		write_bytes += ret;
	} else if (f == read && ret > 0 && !resume_in_progress) {
		read_bytes += ret;
	}
	return ret;
}



int compat20;
char *server_version_string;
char *client_version_string;

char *xxx_host;
struct sockaddr *xxx_hostaddr;

void	 kexdh_client(Kex *);
void	 kexdh_server(Kex *);
void	 kexgex_client(Kex *);
void	 kexgex_server(Kex *);

static int verify_host_key_callback(Key *hostkey);




static int matching_host_key_dns;
struct rdatainfo {
	unsigned int		rdi_length;	/* length of data */
	unsigned char		*rdi_data;	/* record data */
};

struct rrsetinfo {
	unsigned int		rri_flags;	/* RRSET_VALIDATED ... */
	unsigned int		rri_rdclass;	/* class number */
	unsigned int		rri_rdtype;	/* RR type number */
	unsigned int		rri_ttl;	/* time to live */
	unsigned int		rri_nrdatas;	/* size of rdatas array */
	unsigned int		rri_nsigs;	/* size of sigs array */
	char			*rri_name;	/* canonical name */
	struct rdatainfo	*rri_rdatas;	/* individual records */
	struct rdatainfo	*rri_sigs;	/* individual signatures */
};

/* Flags set authorized_keys flags */
int no_port_forwarding_flag;
int no_agent_forwarding_flag;
int no_x11_forwarding_flag;
int no_pty_flag;
int no_user_rc;
int compat13;

#define ANSWER_BUFFER_SIZE 0xffff

# define ERRSET_SUCCESS		0
# define ERRSET_NOMEMORY	1
# define ERRSET_FAIL		2
# define ERRSET_INVAL		3
# define ERRSET_NONAME		4
# define ERRSET_NODATA		5

/*
 * Resolver options (keep these in synch with res_debug.c, please)
 */
#define RES_INIT	0x00000001	/* address initialized */
#define RES_DEBUG	0x00000002	/* print debug messages */
#define RES_AAONLY	0x00000004	/* authoritative answers only (!IMPL)*/
#define RES_USEVC	0x00000008	/* use virtual circuit */
#define RES_PRIMARY	0x00000010	/* query primary server only (!IMPL) */
#define RES_IGNTC	0x00000020	/* ignore trucation errors */
#define RES_RECURSE	0x00000040	/* recursion desired */
#define RES_DEFNAMES	0x00000080	/* use default domain name */
#define RES_STAYOPEN	0x00000100	/* Keep TCP socket open */
#define RES_DNSRCH	0x00000200	/* search up local domain tree */
#define	RES_INSECURE1	0x00000400	/* type 1 security disabled */
#define	RES_INSECURE2	0x00000800	/* type 2 security disabled */
#define	RES_NOALIASES	0x00001000	/* shuts off HOSTALIASES feature */
#define	RES_USE_INET6	0x00002000	/* use/map IPv6 in gethostbyname() */
#define RES_ROTATE	0x00004000	/* rotate ns list after each query */
#define	RES_NOCHECKNAME	0x00008000	/* do not check names for sanity. */
#define	RES_KEEPTSIG	0x00010000	/* do not strip TSIG records */
#define	RES_BLAST	0x00020000	/* blast all recursive servers */
#define RES_USEBSTRING	0x00040000	/* IPv6 reverse lookup with byte
					   strings */
#define RES_NOIP6DOTINT	0x00080000	/* Do not use .ip6.int in IPv6
					   reverse lookup */
#define RES_USE_EDNS0	0x00100000	/* Use EDNS0.  */
#define RES_SNGLKUP	0x00200000	/* one outstanding request at a time */
#define RES_SNGLKUPREOP	0x00400000	/* -"-, but open new socket for each
					   request */
					   
struct dns_query {
	char			*name;
	u_int16_t		type;
	u_int16_t		class;
	struct dns_query	*next;
};

struct dns_rr {
	char			*name;
	u_int16_t		type;
	u_int16_t		class;
	u_int16_t		ttl;
	u_int16_t		size;
	void			*rdata;
	struct dns_rr		*next;
};

struct dns_response {
	int		header;
	struct dns_query	*query;
	struct dns_rr		*answer;
	struct dns_rr		*authority;
	struct dns_rr		*additional;
};

int h_errno;

enum monitor_reqtype {
	MONITOR_REQ_MODULI, MONITOR_ANS_MODULI,
	MONITOR_REQ_FREE, MONITOR_REQ_AUTHSERV,
	MONITOR_REQ_SIGN, MONITOR_ANS_SIGN,
	MONITOR_REQ_PWNAM, MONITOR_ANS_PWNAM,
	MONITOR_REQ_AUTH2_READ_BANNER, MONITOR_ANS_AUTH2_READ_BANNER,
	MONITOR_REQ_AUTHPASSWORD, MONITOR_ANS_AUTHPASSWORD,
	MONITOR_REQ_BSDAUTHQUERY, MONITOR_ANS_BSDAUTHQUERY,
	MONITOR_REQ_BSDAUTHRESPOND, MONITOR_ANS_BSDAUTHRESPOND,
	MONITOR_REQ_SKEYQUERY, MONITOR_ANS_SKEYQUERY,
	MONITOR_REQ_SKEYRESPOND, MONITOR_ANS_SKEYRESPOND,
	MONITOR_REQ_KEYALLOWED, MONITOR_ANS_KEYALLOWED,
	MONITOR_REQ_KEYVERIFY, MONITOR_ANS_KEYVERIFY,
	MONITOR_REQ_KEYEXPORT,
	MONITOR_REQ_PTY, MONITOR_ANS_PTY,
	MONITOR_REQ_PTYCLEANUP,
	MONITOR_REQ_SESSKEY, MONITOR_ANS_SESSKEY,
	MONITOR_REQ_SESSID,
	MONITOR_REQ_RSAKEYALLOWED, MONITOR_ANS_RSAKEYALLOWED,
	MONITOR_REQ_RSACHALLENGE, MONITOR_ANS_RSACHALLENGE,
	MONITOR_REQ_RSARESPONSE, MONITOR_ANS_RSARESPONSE,
	MONITOR_REQ_GSSSETUP, MONITOR_ANS_GSSSETUP,
	MONITOR_REQ_GSSSTEP, MONITOR_ANS_GSSSTEP,
	MONITOR_REQ_GSSUSEROK, MONITOR_ANS_GSSUSEROK,
	MONITOR_REQ_GSSCHECKMIC, MONITOR_ANS_GSSCHECKMIC,
	MONITOR_REQ_PAM_START,
	MONITOR_REQ_PAM_ACCOUNT, MONITOR_ANS_PAM_ACCOUNT,
	MONITOR_REQ_PAM_INIT_CTX, MONITOR_ANS_PAM_INIT_CTX,
	MONITOR_REQ_PAM_QUERY, MONITOR_ANS_PAM_QUERY,
	MONITOR_REQ_PAM_RESPOND, MONITOR_ANS_PAM_RESPOND,
	MONITOR_REQ_PAM_FREE_CTX, MONITOR_ANS_PAM_FREE_CTX,
	MONITOR_REQ_AUDIT_EVENT, MONITOR_REQ_AUDIT_COMMAND,
	MONITOR_REQ_TERM,
	MONITOR_REQ_JPAKE_STEP1, MONITOR_ANS_JPAKE_STEP1,
	MONITOR_REQ_JPAKE_GET_PWDATA, MONITOR_ANS_JPAKE_GET_PWDATA,
	MONITOR_REQ_JPAKE_STEP2, MONITOR_ANS_JPAKE_STEP2,
	MONITOR_REQ_JPAKE_KEY_CONFIRM, MONITOR_ANS_JPAKE_KEY_CONFIRM,
	MONITOR_REQ_JPAKE_CHECK_CONFIRM, MONITOR_ANS_JPAKE_CHECK_CONFIRM,
};

/* Possible values left in `h_errno'.  */
#define	HOST_NOT_FOUND	1	/* Authoritative Answer Host not found.  */
#define	TRY_AGAIN	2	/* Non-Authoritative Host not found,
				   or SERVERFAIL.  */
#define	NO_RECOVERY	3	/* Non recoverable errors, FORMERR, REFUSED,
				   NOTIMP.  */
#define	NO_DATA		4	/* Valid name, no data record of requested
				   type.  */
# define NETDB_INTERNAL	-1	/* See errno.  */
# define NETDB_SUCCESS	0	/* No problem.  */
# define NO_ADDRESS	NO_DATA	/* No address, look for MX record.  */

#define T_RRSIG 46

/* Cipher used for encrypting authentication files. */
#define SSH_AUTHFILE_CIPHER	SSH_CIPHER_3DES

/* Default port number. */
#define SSH_DEFAULT_PORT	22

/* Maximum number of TCP/IP ports forwarded per direction. */
#define SSH_MAX_FORWARDS_PER_DIRECTION	100

/*
 * Maximum number of RSA authentication identity files that can be specified
 * in configuration files or on the command line.
 */
#define SSH_MAX_IDENTITY_FILES		100

#define _PATH_PRIVSEP_CHROOT_DIR	"/var/empty"


int datafellows;

/*
 * Maximum length of lines in authorized_keys file.
 * Current value permits 16kbit RSA and RSA1 keys and 8kbit DSA keys, with
 * some room for options and comments.
 */
#define SSH_MAX_PUBKEY_BYTES		8192
#define TTYSZ 64

struct monitor *pmonitor;
/* Unprivileged user */
struct passwd *privsep_pw;

struct mm_master;
struct monitor {
	int			 m_recvfd;
	int			 m_sendfd;
	struct mm_master	*m_zback;
	struct mm_master	*m_zlib;
	struct Kex		**m_pkex;
	pid_t			 m_pid;
};

struct mon_table {
	enum monitor_reqtype type;
	int flags;
	int (*f)(int, Buffer *);
};

#define MM_MEMSIZE	65536
static char *auth_method = "unknown";

/* variables used for privilege separation */
int use_privsep;


/* message to be displayed after login */
Buffer loginmsg;

/*
 * Any really sensitive data in the application is contained in this
 * structure. The idea is that this structure could be locked into memory so
 * that the pages do not get written into swap.  However, there are some
 * problems. The private key contains BIGNUMs, and we do not (in principle)
 * have access to the internals of them, and locking just the structure is
 * not very useful.  Currently, memory locking is not implemented.
 */
struct {
	Key	*server_key;		/* ephemeral server key */
	Key	*ssh1_host_key;		/* ssh1 host key */
	Key	**host_keys;		/* all private host keys */
	int	have_ssh1_key;
	int	have_ssh2_key;
	u_char	ssh1_cookie[SSH_SESSION_KEY_LENGTH];
} sensitive_data;

/* Converts a private to a public key */
Key *
key_demote(const Key *k)
{
	Key *pk;

	pk = xmalloc(sizeof(*pk));

	return (pk);
}

/* Demote private to public keys for network child */
void
demote_sensitive_data(void)
{
	Key *tmp;
	int i;

	if (sensitive_data.server_key) {
		tmp = key_demote(sensitive_data.server_key);
		key_free(sensitive_data.server_key);
		sensitive_data.server_key = tmp;
	}

	for (i = 0; i < options.num_host_key_files; i++) {
		if (sensitive_data.host_keys[i]) {
			tmp = key_demote(sensitive_data.host_keys[i]);
			key_free(sensitive_data.host_keys[i]);
			sensitive_data.host_keys[i] = tmp;
			if (tmp->type == KEY_RSA1)
				sensitive_data.ssh1_host_key = tmp;
		}
	}

	/* We do not clear ssh1_host key and cookie.  XXX - Okay Niels? */
}

static void
privsep_postauth(Authctxt *authctxt)
{
	u_int32_t rnd[256];
	
	assume(pmonitor != NULL);
	assume(authctxt != NULL);
	assume(authctxt->pw != NULL);
	assume(sensitive_data.host_keys != NULL);
	assume(buffer_size(sensitive_data.host_keys) == options.num_host_key_files*sizeof(Key*));

#ifdef DISABLE_FD_PASSING
	if (1) {
#else
	if (authctxt->pw->pw_uid == 0 || options.use_login) {
#endif
		/* File descriptor passing is broken or root login */
		use_privsep = 0;
		goto skip;
	}

	/* New socket pair */
	monitor_reinit(pmonitor);

	pmonitor->m_pid = fork();
	if (pmonitor->m_pid == -1)
		fatal("fork of unprivileged child failed");
	else if (pmonitor->m_pid != 0) {
		verbose("User child is on pid %ld", (long)pmonitor->m_pid);
		close(pmonitor->m_recvfd);
		buffer_clear(&loginmsg);
		monitor_child_postauth(pmonitor);

		/* NEVERREACHED */
		exit(0);
	}

	close(pmonitor->m_sendfd);

	/* Demote the private keys to public keys. */
	demote_sensitive_data();

	arc4random_stir();
	arc4random_buf(rnd, sizeof(rnd));
	RAND_seed(rnd, sizeof(rnd));

	/* Drop privileges */
	do_setusercontext(authctxt->pw);

 skip:
	/* It is safe now to apply the key state */
	monitor_apply_keystate(pmonitor);

	/*
	 * Tell the packet layer that authentication was successful, since
	 * this information is not part of the key state.
	 */
	packet_set_authenticated();
}
