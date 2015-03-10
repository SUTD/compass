// SSH/auth.c auth_root_allowed

#include "stdlib.h"

void specify_checks()
{
	check_null();
	check_buffer();
}

typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned int u_int;

 
 //---------------------------------------


// ------defines.h---------

typedef int sig_atomic_t;
//----------------------------

/* Protection bits.  */

#define	__S_ISUID	04000	/* Set user ID on execution.  */
#define	__S_ISGID	02000	/* Set group ID on execution.  */
#define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define	__S_IREAD	0400	/* Read by owner.  */
#define	__S_IWRITE	0200	/* Write by owner.  */
#define	__S_IEXEC	0100	/* Execute by owner.  */

/* Protection bits.  */

#define	S_ISUID __S_ISUID	/* Set user ID on execution.  */
#define	S_ISGID	__S_ISGID	/* Set group ID on execution.  */

#if defined __USE_BSD || defined __USE_MISC || defined __USE_XOPEN
/* Save swapped text after use (sticky bit).  This is pretty well obsolete.  */
# define S_ISVTX	__S_ISVTX
#endif

#define	S_IRUSR	__S_IREAD	/* Read by owner.  */
#define	S_IWUSR	__S_IWRITE	/* Write by owner.  */
#define	S_IXUSR	__S_IEXEC	/* Execute by owner.  */
/* Read, write, and execute by owner.  */
#define	S_IRWXU	(__S_IREAD|__S_IWRITE|__S_IEXEC)

#if defined __USE_MISC && defined __USE_BSD
# define S_IREAD	S_IRUSR
# define S_IWRITE	S_IWUSR
# define S_IEXEC	S_IXUSR
#endif

#define	S_IRGRP	(S_IRUSR >> 3)	/* Read by group.  */
#define	S_IWGRP	(S_IWUSR >> 3)	/* Write by group.  */
#define	S_IXGRP	(S_IXUSR >> 3)	/* Execute by group.  */
/* Read, write, and execute by group.  */
#define	S_IRWXG	(S_IRWXU >> 3)

#define	S_IROTH	(S_IRGRP >> 3)	/* Read by others.  */
#define	S_IWOTH	(S_IWGRP >> 3)	/* Write by others.  */
#define	S_IXOTH	(S_IXGRP >> 3)	/* Execute by others.  */
/* Read, write, and execute by others.  */
#define	S_IRWXO	(S_IRWXG >> 3)

# define _PATH_BSHELL "/bin/sh"

/*
 * Maximum number of RSA authentication identity files that can be specified
 * in configuration files or on the command line.
 */
#define SSH_MAX_IDENTITY_FILES		100


/* Maximum number of TCP/IP ports forwarded per direction. */
#define SSH_MAX_FORWARDS_PER_DIRECTION	100

#define MAX_SEND_ENV	256

#define MAX_ALLOW_USERS		256	/* Max # users on allow list. */
#define MAX_DENY_USERS		256	/* Max # users on deny list. */
#define MAX_ALLOW_GROUPS	256	/* Max # groups on allow list. */
#define MAX_DENY_GROUPS		256	/* Max # groups on deny list. */
#define MAX_SUBSYSTEMS		256	/* Max # subsystems. */
#define MAX_HOSTKEYS		256	/* Max # hostkeys. */
#define MAX_ACCEPT_ENV		256	/* Max # of env vars. */
#define MAX_MATCH_GROUPS	256	/* Max # of groups for Match. */

#define MAX_PORTS		256	/* Max # ports. */

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
	unsigned int	num_ports;
	unsigned int	ports_from_cmdline;
	int	ports[MAX_PORTS];	/* Port number to listen on. */
	char   *listen_addr;		/* Address on which the server listens.
*/
	struct addrinfo *listen_addrs;	/* Addresses on which the server
listens. */
	int     address_family;		/* Address family used by the server. */
	char   *host_key_files[MAX_HOSTKEYS];	/* Files containing host keys.
*/
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
	int     x11_use_localhost;	/* If true, use localhost for fake X11
server. */
	char   *xauth_location;	/* Location of xauth program */
	int     strict_modes;	/* If true, require string home dir modes. */
	int     tcp_keep_alive;	/* If true, set SO_KEEPALIVE. */
	char   *ciphers;	/* Supported SSH2 ciphers. */
	char   *macs;		/* Supported SSH2 macs. */
	int	protocol;	/* Supported protocol versions. */
	int     gateway_ports;	/* If true, allow remote connects to forwarded
ports. */
	SyslogFacility log_facility;	/* Facility for system logging. */
	LogLevel log_level;	/* Level for system logging. */
	int     rhosts_rsa_authentication;	/* If true, permit rhosts RSA
						 * authentication. */
	int     hostbased_authentication;	/* If true, permit ssh2
hostbased auth */
	int     hostbased_uses_name_from_packet_only; /* experimental */
	int     rsa_authentication;	/* If true, permit RSA authentication.
*/
	int     pubkey_authentication;	/* If true, permit ssh2 pubkey
authentication. */
	int     kerberos_authentication;	/* If true, permit Kerberos
						 * authentication. */
	int     kerberos_or_local_passwd;	/* If true, permit kerberos
						 * and any other password
						 * authentication mechanism,
						 * such as SecurID or
						 * /etc/passwd */
	int     kerberos_ticket_cleanup;	/* If true, destroy ticket
						 * file on logout. */
	int     kerberos_get_afs_token;		/* If true, try to get AFS token
if
						 * authenticated with Kerberos.
*/
	int     gss_authentication;	/* If true, permit GSSAPI authentication
*/
	int     gss_cleanup_creds;	/* If true, destroy cred cache on logout
*/
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
	unsigned int num_allow_users;
	char   *allow_users[MAX_ALLOW_USERS];
	unsigned int num_deny_users;
	char   *deny_users[MAX_DENY_USERS];
	unsigned int num_allow_groups;
	char   *allow_groups[MAX_ALLOW_GROUPS];
	unsigned int num_deny_groups;
	char   *deny_groups[MAX_DENY_GROUPS];

	unsigned int num_subsystems;
	char   *subsystem_name[MAX_SUBSYSTEMS];
	char   *subsystem_command[MAX_SUBSYSTEMS];
	char   *subsystem_args[MAX_SUBSYSTEMS];

	unsigned int num_accept_env;
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

// ---stat.h

struct stat
  {
	unsigned long int st_dev;		/* Device.  */
   	unsigned long int  st_ino;		/* File serial number.	*/
   	unsigned long int st_nlink;			/* Link count.  */
    	unsigned int st_mode;			/* File mode.  */
       __uid_t st_uid;		/* User ID of the file's owner.	*/
       __gid_t st_gid;		/* Group ID of the file's group.*/
       int __pad0;
     	unsigned long int st_rdev;		/* Device number, if device.  */
     	long int st_size;
  };
  
  //---------buffer.h-------------
  typedef struct {
	u_char	*buf;		/* Buffer for data. */
	u_int	 alloc;		/* Number of bytes allocated for data. */
	u_int	 offset;	/* Offset of first byte containing data. */
	u_int	 end;		/* Offset of last byte containing data. */
}       Buffer;
  
  // ---------------auth.h---------------
    
  typedef struct Authctxt Authctxt;
typedef struct Authmethod Authmethod;
typedef struct KbdintDevice KbdintDevice;

struct Authctxt {
	sig_atomic_t	 success;
	int		 authenticated;	/* authenticated and alarms cancelled */
	int		 postponed;	/* authentication needs another step */
	int		 valid;		/* user exists and is allowed to login
*/
	int		 attempt;
	int		 failures;
	int		 force_pwchange;
	char		*user;		/* username sent by the client */
	char		*service;
	struct passwd	*pw;		/* set if 'valid' */
	char		*style;
	void		*kbdintctxt;
	void		*jpake_ctx;
	Buffer		*loginmsg;
	void		*methoddata;
};
//--------log.c-----------

/* More detailed messages (information that does not need to go to the log). */

void
verbose(const char *fmt,...)
{

}


/* Log this message (information that usually should go to the log). */

void
logit(const char *fmt,...)
{

}
//--------------
//-------------servconf.h-----------

/* permit_root_login */
#define	PERMIT_NOT_SET		-1
#define	PERMIT_NO		0
#define	PERMIT_FORCED_ONLY	1
#define	PERMIT_NO_PASSWD	2
#define	PERMIT_YES		3

  //------------------------
/* Server configuration options. */
ServerOptions options;

int use_privsep;

char *forced_command;

/*
 * Check whether root logins are disallowed.
 */
int
auth_root_allowed(char *method)
{
	switch (options.permit_root_login) {
	case PERMIT_YES:
		return 1;
	case PERMIT_NO_PASSWD:
		if (strcmp(method, "password") != 0)
			return 1;
		break;
	case PERMIT_FORCED_ONLY:
		if (forced_command) {
			logit("Root login accepted for forced command.");
			return 1;
		}
		break;
	}
	logit("ROOT LOGIN REFUSED FROM %.200s", get_remote_ipaddr());
	return 0;
}
