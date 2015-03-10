// SSH/ssh.c simplified
#include <sys/param.h>
#include <stdarg.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --------------Makefile defined-----------

#define SSLEAY_VERSION 1

// ------------STUBS--------------
int stderr;

size_t strlen (__const char * s)
{
	int len = 0;
	while(*s != '\0') {
		len++;
		s++;
	}
	return len;
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

//-----------------ciper.h---------------

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

#define CIPHER_ENCRYPT		1
#define CIPHER_DECRYPT		0

typedef struct Cipher Cipher;
typedef struct CipherContext CipherContext;

struct Cipher;
struct CipherContext {
	int	plaintext;
	int evp;
	Cipher *cipher;
};

//-------------------------------------------

// ----------------------misc.h-------------------


/* Common definitions for ssh tunnel device forwarding */
#define SSH_TUNMODE_NO		0x00
#define SSH_TUNMODE_POINTOPOINT	0x01
#define SSH_TUNMODE_ETHERNET	0x02
#define SSH_TUNMODE_DEFAULT	SSH_TUNMODE_POINTOPOINT
#define SSH_TUNMODE_YES		(SSH_TUNMODE_POINTOPOINT|SSH_TUNMODE_ETHERNET)

#define SSH_TUNID_ANY		0x7fffffff
#define SSH_TUNID_ERR		(SSH_TUNID_ANY - 1)
#define SSH_TUNID_MAX		(SSH_TUNID_ANY - 2)

/* Functions to extract or store big-endian words of various sizes */
u_int64_t	get_u64(const void *)
    __attribute__((__bounded__( __minbytes__, 1, 8)));
u_int32_t	get_u32(const void *)
    __attribute__((__bounded__( __minbytes__, 1, 4)));
u_int16_t	get_u16(const void *)
    __attribute__((__bounded__( __minbytes__, 1, 2)));
void		put_u64(void *, u_int64_t)
    __attribute__((__bounded__( __minbytes__, 1, 8)));
void		put_u32(void *, u_int32_t)
    __attribute__((__bounded__( __minbytes__, 1, 4)));
void		put_u16(void *, u_int16_t)
    __attribute__((__bounded__( __minbytes__, 1, 2)));


/* readpass.c */

#define RP_ECHO			0x0001
#define RP_ALLOW_STDIN		0x0002
#define RP_ALLOW_EOF		0x0004
#define RP_USE_ASKPASS		0x0008

//-----------------------------------------------

// ----------------------stat.h --------------------


/* Versions of the `struct stat' data structure.  */
#define _STAT_VER_KERNEL	0

#if __WORDSIZE == 32
# define _STAT_VER_SVR4		2
# define _STAT_VER_LINUX	3

/* i386 versions of the `xmknod' interface.  */
# define _MKNOD_VER_LINUX	1
# define _MKNOD_VER_SVR4	2
# define _MKNOD_VER		_MKNOD_VER_LINUX /* The bits defined below.  */
#else
# define _STAT_VER_LINUX	1

/* x86-64 versions of the `xmknod' interface.  */
# define _MKNOD_VER_LINUX	0
#endif

#define _STAT_VER		_STAT_VER_LINUX

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

#ifdef __USE_LARGEFILE64
/* Note stat64 has the same shape as stat for x86-64.  */
struct stat64
  {
    __dev_t st_dev;		/* Device.  */
#if __WORDSIZE == 64
    __ino64_t st_ino;		/* File serial number.  */
    __nlink_t st_nlink;		/* Link count.  */
    __mode_t st_mode;		/* File mode.  */
#else
    unsigned int __pad1;
    __ino_t __st_ino;			/* 32bit file serial number.	*/
    __mode_t st_mode;			/* File mode.  */
    __nlink_t st_nlink;			/* Link count.  */
#endif
    __uid_t st_uid;		/* User ID of the file's owner.	*/
    __gid_t st_gid;		/* Group ID of the file's group.*/
#if __WORDSIZE == 64
    int __pad0;
    __dev_t st_rdev;		/* Device number, if device.  */
    __off_t st_size;		/* Size of file, in bytes.  */
#else
    __dev_t st_rdev;			/* Device number, if device.  */
    unsigned int __pad2;
    __off64_t st_size;			/* Size of file, in bytes.  */
#endif
    __blksize_t st_blksize;	/* Optimal block size for I/O.  */
    __blkcnt64_t st_blocks;	/* Nr. 512-byte blocks allocated.  */
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
    __ino64_t st_ino;			/* File serial number.		*/
#endif
  };
#endif

/* Tell code we have these members.  */
#define	_STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV
/* Nanosecond resolution time values are supported.  */
#define _STATBUF_ST_NSEC

/* Encoding of the file mode.  */

#define	__S_IFMT	0170000	/* These bits determine file type.  */

/* File types.  */
#define	__S_IFDIR	0040000	/* Directory.  */
#define	__S_IFCHR	0020000	/* Character device.  */
#define	__S_IFBLK	0060000	/* Block device.  */
#define	__S_IFREG	0100000	/* Regular file.  */
#define	__S_IFIFO	0010000	/* FIFO.  */
#define	__S_IFLNK	0120000	/* Symbolic link.  */
#define	__S_IFSOCK	0140000	/* Socket.  */

/* POSIX.1b objects.  Note that these macros always evaluate to zero.  But
   they do it by enforcing the correct use of the macros.  */
#define __S_TYPEISMQ(buf)  ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSEM(buf) ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSHM(buf) ((buf)->st_mode - (buf)->st_mode)

/* Protection bits.  */

#define	__S_ISUID	04000	/* Set user ID on execution.  */
#define	__S_ISGID	02000	/* Set group ID on execution.  */
#define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define	__S_IREAD	0400	/* Read by owner.  */
#define	__S_IWRITE	0200	/* Write by owner.  */
#define	__S_IEXEC	0100	/* Execute by owner.  */

#ifdef __USE_ATFILE
# define UTIME_NOW	((1l << 30) - 1l)
# define UTIME_OMIT	((1l << 30) - 2l)
#endif

//------------------------------------------------------


//-----------clientloop.h--------------------

/* Multiplexing protocol version */
#define SSHMUX_VER			2

/* Multiplexing control protocol flags */
#define SSHMUX_COMMAND_OPEN		1	/* Open new connection */
#define SSHMUX_COMMAND_ALIVE_CHECK	2	/* Check master is alive */
#define SSHMUX_COMMAND_TERMINATE	3	/* Ask master to exit */

#define SSHMUX_FLAG_TTY			(1)	/* Request tty on open */
#define SSHMUX_FLAG_SUBSYS		(1<<1)	/* Subsystem request on open */
#define SSHMUX_FLAG_X11_FWD		(1<<2)	/* Request X11 forwarding */
#define SSHMUX_FLAG_AGENT_FWD		(1<<3)	/* Request agent forwarding */

//-----------------------------------------


//------------------------defines.h--------------

# define SIZE_T_MAX UINT_MAX

#ifndef _PATH_DEVNULL
# define _PATH_DEVNULL "/dev/null"
#endif

//-----------------------------------------------


//-----------------------fcntl.h-------------


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

#ifdef __USE_GNU
# define O_DIRECT	 040000	/* Direct disk access.	*/
# define O_DIRECTORY	0200000	/* Must be a directory.	 */
# define O_NOFOLLOW	0400000	/* Do not follow links.	 */
# define O_NOATIME     01000000 /* Do not set atime.  */
# define O_CLOEXEC     02000000 /* Set close_on_exec.  */
#endif

/* For now Linux has synchronisity options for data and read operations.
   We define the symbols here but let them do the same as O_SYNC since
   this is a superset.	*/
#if defined __USE_POSIX199309 || defined __USE_UNIX98
# define O_DSYNC	O_SYNC	/* Synchronize data.  */
# define O_RSYNC	O_SYNC	/* Synchronize read operations.	 */
#endif

#ifdef __USE_LARGEFILE64
# if __WORDSIZE == 64
#  define O_LARGEFILE	0
# else
#  define O_LARGEFILE	0100000
# endif
#endif

/* Values for the second argument to `fcntl'.  */
#define F_DUPFD		0	/* Duplicate file descriptor.  */
#define F_GETFD		1	/* Get file descriptor flags.  */
#define F_SETFD		2	/* Set file descriptor flags.  */
#define F_GETFL		3	/* Get file status flags.  */
#define F_SETFL		4	/* Set file status flags.  */
#if __WORDSIZE == 64
# define F_GETLK	5	/* Get record locking info.  */
# define F_SETLK	6	/* Set record locking info (non-blocking).  */
# define F_SETLKW	7	/* Set record locking info (blocking).	*/
/* Not necessary, we always have 64-bit offsets.  */
# define F_GETLK64	5	/* Get record locking info.  */
# define F_SETLK64	6	/* Set record locking info (non-blocking).  */
# define F_SETLKW64	7	/* Set record locking info (blocking).	*/
#else
# ifndef __USE_FILE_OFFSET64
#  define F_GETLK	5	/* Get record locking info.  */
#  define F_SETLK	6	/* Set record locking info (non-blocking).  */
#  define F_SETLKW	7	/* Set record locking info (blocking).	*/
# else
#  define F_GETLK	F_GETLK64  /* Get record locking info.	*/
#  define F_SETLK	F_SETLK64  /* Set record locking info (non-blocking).*/
#  define F_SETLKW	F_SETLKW64 /* Set record locking info (blocking).  */
# endif
# define F_GETLK64	12	/* Get record locking info.  */
# define F_SETLK64	13	/* Set record locking info (non-blocking).  */
# define F_SETLKW64	14	/* Set record locking info (blocking).	*/
#endif

#if defined __USE_BSD || defined __USE_UNIX98
# define F_SETOWN	8	/* Get owner of socket (receiver of SIGIO).  */
# define F_GETOWN	9	/* Set owner of socket (receiver of SIGIO).  */
#endif

#ifdef __USE_GNU
# define F_SETSIG	10	/* Set number of signal to be sent.  */
# define F_GETSIG	11	/* Get number of signal to be sent.  */
#endif

#ifdef __USE_GNU
# define F_SETLEASE	1024	/* Set a lease.	 */
# define F_GETLEASE	1025	/* Enquire what lease is active.  */
# define F_NOTIFY	1026	/* Request notfications on a directory.	 */
# define F_DUPFD_CLOEXEC 1030	/* Duplicate file descriptor with
				   close-on-exit set.  */
#endif

/* For F_[GET|SET]FD.  */
#define FD_CLOEXEC	1	/* actually anything with low bit set goes */

/* For posix fcntl() and `l_type' field of a `struct flock' for lockf().  */
#define F_RDLCK		0	/* Read lock.  */
#define F_WRLCK		1	/* Write lock.	*/
#define F_UNLCK		2	/* Remove lock.	 */

/* For old implementation of bsd flock().  */
#define F_EXLCK		4	/* or 3 */
#define F_SHLCK		8	/* or 4 */

#ifdef __USE_BSD
/* Operations for bsd flock(), also used by the kernel implementation.	*/
# define LOCK_SH	1	/* shared lock */
# define LOCK_EX	2	/* exclusive lock */
# define LOCK_NB	4	/* or'd with one of the above to prevent
				   blocking */
# define LOCK_UN	8	/* remove lock */
#endif

#ifdef __USE_GNU
# define LOCK_MAND	32	/* This is a mandatory flock:	*/
# define LOCK_READ	64	/* ... which allows concurrent read operations.	 */
# define LOCK_WRITE	128	/* ... which allows concurrent write operations.  */
# define LOCK_RW	192	/* ... Which allows concurrent read & write operations.	 */
#endif

#ifdef __USE_GNU
/* Types of directory notifications that may be requested with F_NOTIFY.  */
# define DN_ACCESS	0x00000001	/* File accessed.  */
# define DN_MODIFY	0x00000002	/* File modified.  */
# define DN_CREATE	0x00000004	/* File created.  */
# define DN_DELETE	0x00000008	/* File removed.  */
# define DN_RENAME	0x00000010	/* File renamed.  */
# define DN_ATTRIB	0x00000020	/* File changed attibutes.  */
# define DN_MULTISHOT	0x80000000	/* Don't remove notifier.  */
#endif

struct flock
  {
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.	*/
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
#ifndef __USE_FILE_OFFSET64
    __off_t l_start;	/* Offset where the lock begins.  */
    __off_t l_len;	/* Size of the locked area; zero means until EOF.  */
#else
    __off64_t l_start;	/* Offset where the lock begins.  */
    __off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
#endif
    __pid_t l_pid;	/* Process holding the lock.  */
  };

#ifdef __USE_LARGEFILE64
struct flock64
  {
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.	*/
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
    __off64_t l_start;	/* Offset where the lock begins.  */
    __off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
    __pid_t l_pid;	/* Process holding the lock.  */
  };
#endif

/* Define some more compatibility macros to be backward compatible with
   BSD systems which did not managed to hide these kernel macros.  */
#ifdef	__USE_BSD
# define FAPPEND	O_APPEND
# define FFSYNC		O_FSYNC
# define FASYNC		O_ASYNC
# define FNONBLOCK	O_NONBLOCK
# define FNDELAY	O_NDELAY
#endif /* Use BSD.  */

/* Advise to `posix_fadvise'.  */
#ifdef __USE_XOPEN2K
# define POSIX_FADV_NORMAL	0 /* No further special treatment.  */
# define POSIX_FADV_RANDOM	1 /* Expect random page references.  */
# define POSIX_FADV_SEQUENTIAL	2 /* Expect sequential page references.	 */
# define POSIX_FADV_WILLNEED	3 /* Will need these pages.  */
# define POSIX_FADV_DONTNEED	4 /* Don't need these pages.  */
# define POSIX_FADV_NOREUSE	5 /* Data will be accessed once.  */
#endif
// -----------------------------

// ------------------compat.h -----------------

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

// ---------------------------------------------------------


//-----------------socket.h--------------------

/* Type for length arguments in socket calls.  */
#ifndef __socklen_t_defined
typedef __socklen_t socklen_t;
# define __socklen_t_defined
#endif

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

/* Protocol families.  */
#define	PF_UNSPEC	0	/* Unspecified.  */
#define	PF_LOCAL	1	/* Local to host (pipes and file-domain).  */
#define	PF_UNIX		PF_LOCAL /* POSIX name for PF_LOCAL.  */
#define	PF_FILE		PF_LOCAL /* Another non-standard name for PF_LOCAL.  */
#define	PF_INET		2	/* IP protocol family.  */
#define	PF_AX25		3	/* Amateur Radio AX.25.  */
#define	PF_IPX		4	/* Novell Internet Protocol.  */
#define	PF_APPLETALK	5	/* Appletalk DDP.  */
#define	PF_NETROM	6	/* Amateur radio NetROM.  */
#define	PF_BRIDGE	7	/* Multiprotocol bridge.  */
#define	PF_ATMPVC	8	/* ATM PVCs.  */
#define	PF_X25		9	/* Reserved for X.25 project.  */
#define	PF_INET6	10	/* IP version 6.  */
#define	PF_ROSE		11	/* Amateur Radio X.25 PLP.  */
#define	PF_DECnet	12	/* Reserved for DECnet project.  */
#define	PF_NETBEUI	13	/* Reserved for 802.2LLC project.  */
#define	PF_SECURITY	14	/* Security callback pseudo AF.  */
#define	PF_KEY		15	/* PF_KEY key management API.  */
#define	PF_NETLINK	16
#define	PF_ROUTE	PF_NETLINK /* Alias to emulate 4.4BSD.  */
#define	PF_PACKET	17	/* Packet family.  */
#define	PF_ASH		18	/* Ash.  */
#define	PF_ECONET	19	/* Acorn Econet.  */
#define	PF_ATMSVC	20	/* ATM SVCs.  */
#define PF_RDS		21	/* RDS sockets.  */
#define	PF_SNA		22	/* Linux SNA Project */
#define	PF_IRDA		23	/* IRDA sockets.  */
#define	PF_PPPOX	24	/* PPPoX sockets.  */
#define	PF_WANPIPE	25	/* Wanpipe API sockets.  */
#define PF_LLC		26	/* Linux LLC.  */
#define PF_CAN		29	/* Controller Area Network.  */
#define PF_TIPC		30	/* TIPC sockets.  */
#define	PF_BLUETOOTH	31	/* Bluetooth sockets.  */
#define	PF_IUCV		32	/* IUCV sockets.  */
#define PF_RXRPC	33	/* RxRPC sockets.  */
#define PF_ISDN		34	/* mISDN sockets.  */
#define PF_PHONET	35	/* Phonet sockets.  */
#define PF_IEEE802154	36	/* IEEE 802.15.4 sockets.  */
#define	PF_MAX		37	/* For now..  */

/* Address families.  */
#define	AF_UNSPEC	PF_UNSPEC
#define	AF_LOCAL	PF_LOCAL
#define	AF_UNIX		PF_UNIX
#define	AF_FILE		PF_FILE
#define	AF_INET		PF_INET
#define	AF_AX25		PF_AX25
#define	AF_IPX		PF_IPX
#define	AF_APPLETALK	PF_APPLETALK
#define	AF_NETROM	PF_NETROM
#define	AF_BRIDGE	PF_BRIDGE
#define	AF_ATMPVC	PF_ATMPVC
#define	AF_X25		PF_X25
#define	AF_INET6	PF_INET6
#define	AF_ROSE		PF_ROSE
#define	AF_DECnet	PF_DECnet
#define	AF_NETBEUI	PF_NETBEUI
#define	AF_SECURITY	PF_SECURITY
#define	AF_KEY		PF_KEY
#define	AF_NETLINK	PF_NETLINK
#define	AF_ROUTE	PF_ROUTE
#define	AF_PACKET	PF_PACKET
#define	AF_ASH		PF_ASH
#define	AF_ECONET	PF_ECONET
#define	AF_ATMSVC	PF_ATMSVC
#define AF_RDS		PF_RDS
#define	AF_SNA		PF_SNA
#define	AF_IRDA		PF_IRDA
#define	AF_PPPOX	PF_PPPOX
#define	AF_WANPIPE	PF_WANPIPE
#define AF_LLC		PF_LLC
#define AF_CAN		PF_CAN
#define AF_TIPC		PF_TIPC
#define	AF_BLUETOOTH	PF_BLUETOOTH
#define	AF_IUCV		PF_IUCV
#define AF_RXRPC	PF_RXRPC
#define AF_ISDN		PF_ISDN
#define AF_PHONET	PF_PHONET
#define AF_IEEE802154	PF_IEEE802154
#define	AF_MAX		PF_MAX

/* Socket level values.  Others are defined in the appropriate headers.

   XXX These definitions also should go into the appropriate headers as
   far as they are available.  */
#define SOL_RAW		255
#define SOL_DECNET      261
#define SOL_X25         262
#define SOL_PACKET	263
#define SOL_ATM		264	/* ATM layer (cell level).  */
#define SOL_AAL		265	/* ATM Adaption Layer (packet level).  */
#define SOL_IRDA	266

/* Maximum queue length specifiable by listen.  */
#define SOMAXCONN	128

/* Get the definition of the macro to define the common sockaddr members.  */
#include <bits/sockaddr.h>

/* Structure describing a generic socket address.  */
struct sockaddr
  {
    __SOCKADDR_COMMON (sa_);	/* Common data: address family and length.  */
    char sa_data[14];		/* Address data.  */
  };


/* Structure large enough to hold any socket address (with the historical
   exception of AF_UNIX).  We reserve 128 bytes.  */
#define __ss_aligntype	unsigned long int
#define _SS_SIZE	128
#define _SS_PADSIZE	(_SS_SIZE - (2 * sizeof (__ss_aligntype)))

struct sockaddr_storage
  {
    __SOCKADDR_COMMON (ss_);	/* Address family, etc.  */
    __ss_aligntype __ss_align;	/* Force desired alignment.  */
    char __ss_padding[_SS_PADSIZE];
  };


/* Bits in the FLAGS argument to `send', `recv', et al.  */
enum
  {
    MSG_OOB		= 0x01,	/* Process out-of-band data.  */
#define MSG_OOB		MSG_OOB
    MSG_PEEK		= 0x02,	/* Peek at incoming messages.  */
#define MSG_PEEK	MSG_PEEK
    MSG_DONTROUTE	= 0x04,	/* Don't use local routing.  */
#define MSG_DONTROUTE	MSG_DONTROUTE
#ifdef __USE_GNU
    /* DECnet uses a different name.  */
    MSG_TRYHARD		= MSG_DONTROUTE,
# define MSG_TRYHARD	MSG_DONTROUTE
#endif
    MSG_CTRUNC		= 0x08,	/* Control data lost before delivery.  */
#define MSG_CTRUNC	MSG_CTRUNC
    MSG_PROXY		= 0x10,	/* Supply or ask second address.  */
#define MSG_PROXY	MSG_PROXY
    MSG_TRUNC		= 0x20,
#define	MSG_TRUNC	MSG_TRUNC
    MSG_DONTWAIT	= 0x40, /* Nonblocking IO.  */
#define	MSG_DONTWAIT	MSG_DONTWAIT
    MSG_EOR		= 0x80, /* End of record.  */
#define	MSG_EOR		MSG_EOR
    MSG_WAITALL		= 0x100, /* Wait for a full request.  */
#define	MSG_WAITALL	MSG_WAITALL
    MSG_FIN		= 0x200,
#define	MSG_FIN		MSG_FIN
    MSG_SYN		= 0x400,
#define	MSG_SYN		MSG_SYN
    MSG_CONFIRM		= 0x800, /* Confirm path validity.  */
#define	MSG_CONFIRM	MSG_CONFIRM
    MSG_RST		= 0x1000,
#define	MSG_RST		MSG_RST
    MSG_ERRQUEUE	= 0x2000, /* Fetch message from error queue.  */
#define	MSG_ERRQUEUE	MSG_ERRQUEUE
    MSG_NOSIGNAL	= 0x4000, /* Do not generate SIGPIPE.  */
#define	MSG_NOSIGNAL	MSG_NOSIGNAL
    MSG_MORE		= 0x8000,  /* Sender will send more.  */
#define	MSG_MORE	MSG_MORE

    MSG_CMSG_CLOEXEC	= 0x40000000	/* Set close_on_exit for file
                                           descriptor received through
                                           SCM_RIGHTS.  */
#define MSG_CMSG_CLOEXEC MSG_CMSG_CLOEXEC
  };


/* Structure describing messages sent by
   `sendmsg' and received by `recvmsg'.  */
struct msghdr
  {
    void *msg_name;		/* Address to send to/receive from.  */
    socklen_t msg_namelen;	/* Length of address data.  */

    struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
    size_t msg_iovlen;		/* Number of elements in the vector.  */

    void *msg_control;		/* Ancillary data (eg BSD filedesc passing). */
    size_t msg_controllen;	/* Ancillary data buffer length.
				   !! The type should be socklen_t but the
				   definition of the kernel is incompatible
				   with this.  */

    int msg_flags;		/* Flags on received message.  */
  };

/* Structure used for storage of ancillary data object information.  */
struct cmsghdr
  {
    size_t cmsg_len;		/* Length of data in cmsg_data plus length
				   of cmsghdr structure.
				   !! The type should be socklen_t but the
				   definition of the kernel is incompatible
				   with this.  */
    int cmsg_level;		/* Originating protocol.  */
    int cmsg_type;		/* Protocol specific type.  */
#if (!defined __STRICT_ANSI__ && __GNUC__ >= 2) || __STDC_VERSION__ >= 199901L
    __extension__ unsigned char __cmsg_data __flexarr; /* Ancillary data.  */
#endif
  };

/* Ancillary data object manipulation macros.  */
#if (!defined __STRICT_ANSI__ && __GNUC__ >= 2) || __STDC_VERSION__ >= 199901L
# define CMSG_DATA(cmsg) ((cmsg)->__cmsg_data)
#else
# define CMSG_DATA(cmsg) ((unsigned char *) ((struct cmsghdr *) (cmsg) + 1))
#endif
#define CMSG_NXTHDR(mhdr, cmsg) __cmsg_nxthdr (mhdr, cmsg)
#define CMSG_FIRSTHDR(mhdr) \
  ((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr)		      \
   ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) 0)
#define CMSG_ALIGN(len) (((len) + sizeof (size_t) - 1) \
			 & (size_t) ~(sizeof (size_t) - 1))
#define CMSG_SPACE(len) (CMSG_ALIGN (len) \
			 + CMSG_ALIGN (sizeof (struct cmsghdr)))
#define CMSG_LEN(len)   (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))

extern struct cmsghdr *__cmsg_nxthdr (struct msghdr *__mhdr,
				      struct cmsghdr *__cmsg) __THROW;
#ifdef __USE_EXTERN_INLINES
# ifndef _EXTERN_INLINE
#  define _EXTERN_INLINE __extern_inline
# endif
_EXTERN_INLINE struct cmsghdr *
__NTH (__cmsg_nxthdr (struct msghdr *__mhdr, struct cmsghdr *__cmsg))
{
  if ((size_t) __cmsg->cmsg_len < sizeof (struct cmsghdr))
    /* The kernel header does this so there may be a reason.  */
    return 0;

  __cmsg = (struct cmsghdr *) ((unsigned char *) __cmsg
			       + CMSG_ALIGN (__cmsg->cmsg_len));
  if ((unsigned char *) (__cmsg + 1) > ((unsigned char *) __mhdr->msg_control
					+ __mhdr->msg_controllen)
      || ((unsigned char *) __cmsg + CMSG_ALIGN (__cmsg->cmsg_len)
	  > ((unsigned char *) __mhdr->msg_control + __mhdr->msg_controllen)))
    /* No more entries.  */
    return 0;
  return __cmsg;
}
#endif	/* Use `extern inline'.  */

/* Socket level message types.  This must match the definitions in
   <linux/socket.h>.  */
enum
  {
    SCM_RIGHTS = 0x01		/* Transfer file descriptors.  */
#define SCM_RIGHTS SCM_RIGHTS
#ifdef __USE_GNU
    , SCM_CREDENTIALS = 0x02	/* Credentials passing.  */
# define SCM_CREDENTIALS SCM_CREDENTIALS
#endif
  };

#ifdef __USE_GNU
/* User visible structure for SCM_CREDENTIALS message */
struct ucred
{
  pid_t pid;			/* PID of sending process.  */
  uid_t uid;			/* UID of sending process.  */
  gid_t gid;			/* GID of sending process.  */
};
#endif

/* Ugly workaround for unclean kernel headers.  */
#if !defined __USE_MISC && !defined __USE_GNU
# ifndef FIOGETOWN
#  define __SYS_SOCKET_H_undef_FIOGETOWN
# endif
# ifndef FIOSETOWN
#  define __SYS_SOCKET_H_undef_FIOSETOWN
# endif
# ifndef SIOCATMARK
#  define __SYS_SOCKET_H_undef_SIOCATMARK
# endif
# ifndef SIOCGPGRP
#  define __SYS_SOCKET_H_undef_SIOCGPGRP
# endif
# ifndef SIOCGSTAMP
#  define __SYS_SOCKET_H_undef_SIOCGSTAMP
# endif
# ifndef SIOCGSTAMPNS
#  define __SYS_SOCKET_H_undef_SIOCGSTAMPNS
# endif
# ifndef SIOCSPGRP
#  define __SYS_SOCKET_H_undef_SIOCSPGRP
# endif
#endif

/* Get socket manipulation related informations from kernel headers.  */
#include <asm/socket.h>

#if !defined __USE_MISC && !defined __USE_GNU
# ifdef __SYS_SOCKET_H_undef_FIOGETOWN
#  undef __SYS_SOCKET_H_undef_FIOGETOWN
#  undef FIOGETOWN
# endif
# ifdef __SYS_SOCKET_H_undef_FIOSETOWN
#  undef __SYS_SOCKET_H_undef_FIOSETOWN
#  undef FIOSETOWN
# endif
# ifdef __SYS_SOCKET_H_undef_SIOCATMARK
#  undef __SYS_SOCKET_H_undef_SIOCATMARK
#  undef SIOCATMARK
# endif
# ifdef __SYS_SOCKET_H_undef_SIOCGPGRP
#  undef __SYS_SOCKET_H_undef_SIOCGPGRP
#  undef SIOCGPGRP
# endif
# ifdef __SYS_SOCKET_H_undef_SIOCGSTAMP
#  undef __SYS_SOCKET_H_undef_SIOCGSTAMP
#  undef SIOCGSTAMP
# endif
# ifdef __SYS_SOCKET_H_undef_SIOCGSTAMPNS
#  undef __SYS_SOCKET_H_undef_SIOCGSTAMPNS
#  undef SIOCGSTAMPNS
# endif
# ifdef __SYS_SOCKET_H_undef_SIOCSPGRP
#  undef __SYS_SOCKET_H_undef_SIOCSPGRP
#  undef SIOCSPGRP
# endif
#endif

/* Structure used to manipulate the SO_LINGER option.  */
struct linger
  {
    int l_onoff;		/* Nonzero to linger on close.  */
    int l_linger;		/* Time to linger.  */
  };
  
  //----------------------------------------------------


//-------------buffer.h -------------------------

typedef struct {
	u_char	*buf;		/* Buffer for data. */
	u_int	 alloc;		/* Number of bytes allocated for data. */
	u_int	 offset;	/* Offset of first byte containing data. */
	u_int	 end;		/* Offset of last byte containing data. */
}       Buffer;

//---------------------------------------------------

// ------------------key.h -------------------------

typedef int* RSA;
typedef int* DSA;

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

// --------------------------------------------------------------

// ----------------ssh.h------------------------

/* $OpenBSD: ssh.h,v 1.78 2006/08/03 03:34:42 deraadt Exp $ */

/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

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

/*
 * Maximum length of lines in authorized_keys file.
 * Current value permits 16kbit RSA and RSA1 keys and 8kbit DSA keys, with
 * some room for options and comments.
 */
#define SSH_MAX_PUBKEY_BYTES		8192

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

/*
 * Name for the service.  The port named by this service overrides the
 * default port if present.
 */
#define SSH_SERVICE_NAME	"ssh"

/*
 * Name of the environment variable containing the process ID of the
 * authentication agent.
 */
#define SSH_AGENTPID_ENV_NAME	"SSH_AGENT_PID"

/*
 * Name of the environment variable containing the pathname of the
 * authentication socket.
 */
#define SSH_AUTHSOCKET_ENV_NAME "SSH_AUTH_SOCK"

/*
 * Environment variable for overwriting the default location of askpass
 */
#define SSH_ASKPASS_ENV		"SSH_ASKPASS"

/*
 * Force host key length and server key length to differ by at least this
 * many bits.  This is to make double encryption with rsaref work.
 */
#define SSH_KEY_BITS_RESERVED		128

/*
 * Length of the session key in bytes.  (Specified as 256 bits in the
 * protocol.)
 */
#define SSH_SESSION_KEY_LENGTH		32

/* Used to identify ``EscapeChar none'' */
#define SSH_ESCAPECHAR_NONE		-2

/*
 * unprivileged user when UsePrivilegeSeparation=yes;
 * sshd will change its privileges to this user and its
 * primary group.
 */
#ifndef SSH_PRIVSEP_USER
#define SSH_PRIVSEP_USER		"sshd"
#endif

/* Minimum modulus size (n) for RSA keys. */
#define SSH_RSA_MINIMUM_MODULUS_SIZE	768

/* Listen backlog for sshd, ssh-agent and forwarding sockets */
#define SSH_LISTEN_BACKLOG		128

// -------------------------------------------------------------------------------------

// ----------------------log.h ------------------------------------

/* $OpenBSD: log.h,v 1.17 2008/06/13 00:12:02 dtucker Exp $ */

/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */


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
// ---------------------------------------------------------

//----------------ssh-connect.h------------------

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

//------------------------------------------------------------------


//-----------------------------readconf.h------------------


/* Data structure for representing a forwarding request. */

typedef struct {
	char	 *listen_host;		/* Host (address) to listen on. */
	int	  listen_port;		/* Port to forward. */
	char	 *connect_host;		/* Host to connect. */
	int	  connect_port;		/* Port to connect on connect_host. */
}       Forward;
/* Data structure for representing option data. */

#define MAX_SEND_ENV	256

typedef struct {
	int     forward_agent;	/* Forward authentication agent. */
	int     forward_x11;	/* Forward X11 display. */
	int     forward_x11_trusted;	/* Trust Forward X11 display. */
	int     exit_on_forward_failure;	/* Exit if bind(2) fails for -L/-R */
	char   *xauth_location;	/* Location for xauth program */
	int     gateway_ports;	/* Allow remote connects to forwarded ports. */
	int     use_privileged_port;	/* Don't use privileged port if false. */
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
	int     kbd_interactive_authentication; /* Try keyboard-interactive auth. */
	char	*kbd_interactive_devices; /* Keyboard-interactive auth devices. */
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
	char   *hostkeyalgorithms;	/* SSH2 server key types in order of preference. */
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
	char   *bind_address;	/* local socket address for connection to sshd */
	char   *smartcard_device; /* Smartcard reader device */
	int	verify_host_key_dns;	/* Verify host key using DNS */

	int     num_identity_files;	/* Number of files for RSA/DSA identities. */
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

#define SSHCTL_MASTER_NO	0
#define SSHCTL_MASTER_YES	1
#define SSHCTL_MASTER_AUTO	2
#define SSHCTL_MASTER_ASK	3
#define SSHCTL_MASTER_AUTO_ASK	4

// -------------------------------------------------------------------------


// ----------------limits.h ------------------

/* This administrivia gets added to the beginning of limits.h
   if the system has its own version of limits.h.  */

/* We use _GCC_LIMITS_H_ because we want this not to match
   any macros that the system's limits.h uses for its own purposes.  */
#ifndef _GCC_LIMITS_H_  /* Terminated in limity.h.  */
#define _GCC_LIMITS_H_

#ifndef _LIBC_LIMITS_H_
/* Use "..." so that we find syslimits.h only in this same directory.  */
#include "syslimits.h"
#endif
#ifndef _LIMITS_H___
#define _LIMITS_H___

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

/* Minimum and maximum values a `signed short int' can hold.  */
#undef SHRT_MIN
#define SHRT_MIN (-SHRT_MAX - 1)
#undef SHRT_MAX
#define SHRT_MAX __SHRT_MAX__

/* Maximum value an `unsigned short int' can hold.  (Minimum is 0).  */
#undef USHRT_MAX
#if __SHRT_MAX__ == __INT_MAX__
# define USHRT_MAX (SHRT_MAX * 2U + 1U)
#else
# define USHRT_MAX (SHRT_MAX * 2 + 1)
#endif

/* Minimum and maximum values a `signed int' can hold.  */
#undef INT_MIN
#define INT_MIN (-INT_MAX - 1)
#undef INT_MAX
#define INT_MAX __INT_MAX__

/* Maximum value an `unsigned int' can hold.  (Minimum is 0).  */
#undef UINT_MAX
#define UINT_MAX (INT_MAX * 2U + 1U)

/* Minimum and maximum values a `signed long int' can hold.
   (Same as `int').  */
#undef LONG_MIN
#define LONG_MIN (-LONG_MAX - 1L)
#undef LONG_MAX
#define LONG_MAX __LONG_MAX__

/* Maximum value an `unsigned long int' can hold.  (Minimum is 0).  */
#undef ULONG_MAX
#define ULONG_MAX (LONG_MAX * 2UL + 1UL)

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* Minimum and maximum values a `signed long long int' can hold.  */
# undef LLONG_MIN
# define LLONG_MIN (-LLONG_MAX - 1LL)
# undef LLONG_MAX
# define LLONG_MAX __LONG_LONG_MAX__

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0).  */
# undef ULLONG_MAX
# define ULLONG_MAX (LLONG_MAX * 2ULL + 1ULL)
#endif

#if defined (__GNU_LIBRARY__) ? defined (__USE_GNU) : !defined (__STRICT_ANSI__)
/* Minimum and maximum values a `signed long long int' can hold.  */
# undef LONG_LONG_MIN
# define LONG_LONG_MIN (-LONG_LONG_MAX - 1LL)
# undef LONG_LONG_MAX
# define LONG_LONG_MAX __LONG_LONG_MAX__

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0).  */
# undef ULONG_LONG_MAX
# define ULONG_LONG_MAX (LONG_LONG_MAX * 2ULL + 1ULL)
#endif

#endif /* _LIMITS_H___ */
/* This administrivia gets added to the end of limits.h
   if the system has its own version of limits.h.  */

#else /* not _GCC_LIMITS_H_ */

#ifdef _GCC_NEXT_LIMITS_H
#include_next <limits.h>		/* recurse down to the real one */
#endif

#endif /* not _GCC_LIMITS_H_ */

// -----------------------------------------------------------------------

//----------------------version.h---------------------------

#define SSH_VERSION	"OpenSSH_5.3"

#define SSH_PORTABLE	"p1"
#define SSH_RELEASE	SSH_VERSION SSH_PORTABLE

//----------------------------------------------------------------


// ----------------------pwd.h -------------------------


#if defined __USE_XOPEN || defined __USE_XOPEN2K
/* The Single Unix specification says that some more types are
   available here.  */
# ifndef __gid_t_defined
typedef __gid_t gid_t;
#  define __gid_t_defined
# endif

# ifndef __uid_t_defined
typedef __uid_t uid_t;
#  define __uid_t_defined
# endif
#endif

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



// ---------------------------------------------------------


// --------------------------ssh-connect.c-----------------

typedef struct Sensitive Sensitive;
struct Sensitive {
	Key	**keys;
	int	nkeys;
	int	external_keysign;
};

// -----------------------------------------------

// ------------fatal--------------

void
fatal(const char *fmt,...)
{
	exit(1);
}


// --------------------------xmalloc.c --------------------

/* $OpenBSD: xmalloc.c,v 1.27 2006/08/03 03:34:42 deraadt Exp $ */
/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Versions of malloc and friends that check their results, and never return
 * failure (they call fatal if they encounter an error).
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */


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

void *
xrealloc(void *ptr, size_t nmemb, size_t size)
{
	void *new_ptr;
	size_t new_size = nmemb * size;

	if (new_size == 0)
		fatal("xrealloc: zero size");
	if (SIZE_T_MAX / nmemb < size)
		fatal("xrealloc: nmemb * size > SIZE_T_MAX");
	if (ptr == NULL)
		new_ptr = malloc(new_size);
	else
		new_ptr = realloc(ptr, new_size);
	if (new_ptr == NULL)
		fatal("xrealloc: out of memory (new_size %lu bytes)",
		    (u_long) new_size);
	return new_ptr;
}

void
xfree(void *ptr)
{
	if (ptr == NULL)
		fatal("xfree: NULL pointer given as argument");
	free(ptr);
}

char *
xstrdup(const char *str)
{
	size_t len;
	char *cp;

	len = strlen(str) + 1;
	cp = xmalloc(len);
	strlcpy(cp, str, len);
	return cp;
}

int
xasprintf(char **ret, const char *fmt, ...)
{
	va_list ap;
	int i;

	va_start(ap, fmt);
	i = vasprintf(ret, fmt, ap);
	va_end(ap);

	if (i < 0 || *ret == NULL)
		fatal("xasprintf: could not allocate memory");

	return (i);
}
// ----------------------------------------


// ----------------misc.c-----------------------
int errno;
void
sanitise_stdfd(void)
{
	int nullfd, dupfd;

	if ((nullfd = dupfd = open(_PATH_DEVNULL, O_RDWR)) == -1) {
		fprintf(stderr, "Couldn't open /dev/null: %s\n",
		    strerror(errno));
		exit(1);
	}
	while (++dupfd <= 2) {
		// Only clobber closed fds 
		if (fcntl(dupfd, F_GETFL, 0) >= 0)
			continue;
		if (dup2(nullfd, dupfd) == -1) {
			fprintf(stderr, "dup2: %s\n", strerror(errno));
			exit(1);
		}
	}
	if (nullfd > 2)
		close(nullfd);
	
}

struct passwd *
pwcopy(struct passwd *pw)
{
	struct passwd *copy = xcalloc(1, sizeof(*copy));

	copy->pw_name = xstrdup(pw->pw_name);
	copy->pw_passwd = xstrdup(pw->pw_passwd);
	copy->pw_gecos = xstrdup(pw->pw_gecos);
	copy->pw_uid = pw->pw_uid;
	copy->pw_gid = pw->pw_gid;
#ifdef HAVE_PW_EXPIRE_IN_PASSWD
	copy->pw_expire = pw->pw_expire;
#endif
#ifdef HAVE_PW_CHANGE_IN_PASSWD
	copy->pw_change = pw->pw_change;
#endif
#ifdef HAVE_PW_CLASS_IN_PASSWD
	copy->pw_class = xstrdup(pw->pw_class);
#endif
	copy->pw_dir = xstrdup(pw->pw_dir);
	copy->pw_shell = xstrdup(pw->pw_shell);
	return copy;
}


// -----------------------------------------------------------

// ------------------readconf.c-------------------------

/*
 * Initializes options to special values that indicate that they have not yet
 * been set.  Read_config_file will only set options with this value. Options
 * are processed in the following order: command line, user config file,
 * system config file.  Last, fill_default_options is called.
 */

void
initialize_options(Options * options)
{
	memset(options, 'X', sizeof(*options));
	options->forward_agent = -1;
	options->forward_x11 = -1;
	options->forward_x11_trusted = -1;
	options->exit_on_forward_failure = -1;
	options->xauth_location = NULL;
	options->gateway_ports = -1;
	options->use_privileged_port = -1;
	options->rsa_authentication = -1;
	options->pubkey_authentication = -1;
	options->challenge_response_authentication = -1;
	options->gss_authentication = -1;
	options->gss_deleg_creds = -1;
	options->password_authentication = -1;
	options->kbd_interactive_authentication = -1;
	options->kbd_interactive_devices = NULL;
	options->rhosts_rsa_authentication = -1;
	options->hostbased_authentication = -1;
	options->batch_mode = -1;
	options->check_host_ip = -1;
	options->strict_host_key_checking = -1;
	options->compression = -1;
	options->tcp_keep_alive = -1;
	options->compression_level = -1;
	options->port = -1;
	options->address_family = -1;
	options->connection_attempts = -1;
	options->connection_timeout = -1;
	options->number_of_password_prompts = -1;
	options->cipher = -1;
	options->ciphers = NULL;
	options->macs = NULL;
	options->hostkeyalgorithms = NULL;
	options->protocol = SSH_PROTO_UNKNOWN;
	options->num_identity_files = 0;
	options->hostname = NULL;
	options->host_key_alias = NULL;
	options->proxy_command = NULL;
	options->user = NULL;
	options->escape_char = -1;
	options->system_hostfile = NULL;
	options->user_hostfile = NULL;
	options->system_hostfile2 = NULL;
	options->user_hostfile2 = NULL;
	options->num_local_forwards = 0;
	options->num_remote_forwards = 0;
	options->clear_forwardings = -1;
	options->log_level = SYSLOG_LEVEL_NOT_SET;
	options->preferred_authentications = NULL;
	options->bind_address = NULL;
	options->smartcard_device = NULL;
	options->enable_ssh_keysign = - 1;
	options->no_host_authentication_for_localhost = - 1;
	options->identities_only = - 1;
	options->rekey_limit = - 1;
	options->verify_host_key_dns = -1;
	options->server_alive_interval = -1;
	options->server_alive_count_max = -1;
	options->num_send_env = 0;
	options->control_path = NULL;
	options->control_master = -1;
	options->hash_known_hosts = -1;
	options->tun_open = -1;
	options->tun_local = -1;
	options->tun_remote = -1;
	options->local_command = NULL;
	options->permit_local_command = -1;
	options->use_roaming = -1;
	options->visual_host_key = -1;
	options->zero_knowledge_password_authentication = -1; 
}

// ----------------------------------------------------

//-----------------------bsd-misc.c-----------

/*
 * NB. duplicate __progname in case it is an alias for argv[0]
 * Otherwise it may get clobbered by setproctitle()
 */
char *ssh_get_progname(char *argv0)
{
#ifdef HAVE___PROGNAME
	extern char *__progname;

	return xstrdup(__progname);
#else
	char *p;

	if (argv0 == NULL)
		return ("unknown");	/* XXX */
	p = strrchr(argv0, '/');
	if (p == NULL)
		p = argv0;
	else
		p++;

	return (xstrdup(p));
#endif
}

// ----------------------------------------------------

// -----------------------ssh.c --------------------------------------

extern char *__progname;

/* Flag indicating whether debug mode is on.  May be set on the command line. */
int debug_flag = 0;

/* Flag indicating whether a tty should be allocated */
int tty_flag = 0;
int no_tty_flag = 0;
int force_tty_flag = 0;

/* don't exec a shell */
int no_shell_flag = 0;

/*
 * Flag indicating that nothing should be read from stdin.  This can be set
 * on the command line.
 */
int stdin_null_flag = 0;

/*
 * Flag indicating that ssh should fork after authentication.  This is useful
 * so that the passphrase can be entered manually, and then ssh goes to the
 * background.
 */
int fork_after_authentication_flag = 0;

/*
 * General data structure for command line options and options configurable
 * in configuration files.  See readconf.h.
 */
Options options;

/* optional user configfile */
char *config = NULL;

/*
 * Name of the host we are connecting to.  This is the name given on the
 * command line, or the HostName specified for the user-supplied name in a
 * configuration file.
 */
char *host;

/* socket address the host resolves to */
struct sockaddr_storage hostaddr;

/* Private host keys. */
Sensitive sensitive_data;

/* Original real UID. */
uid_t original_real_uid;
uid_t original_effective_uid;

/* command to be executed */
Buffer command;

/* Should we execute a command or invoke a subsystem? */
int subsystem_flag = 0;

/* # of replies received for global requests */
static int remote_forward_confirms_received = 0;

/* pid of proxycommand child process */
pid_t proxy_command_pid = 0;

/* mux.c */
extern int muxserver_sock;
extern u_int muxclient_command;

/* Prints a help message to the user.  This function never returns. */

static void
usage(void)
{
	fprintf(stderr,
"usage: ssh [-1246AaCfgKkMNnqsTtVvXxYy] [-b bind_address] [-c cipher_spec]\n"
"           [-D [bind_address:]port] [-e escape_char] [-F configfile]\n"
"           [-i identity_file] [-L [bind_address:]port:host:hostport]\n"
"           [-l login_name] [-m mac_spec] [-O ctl_cmd] [-o option] [-p port]\n"
"           [-R [bind_address:]port:host:hostport] [-S ctl_path]\n"
"           [-w local_tun[:remote_tun]] [user@]hostname [command]\n"
	);
	exit(255);
}

void* getpwuid(int x);

/*
 * Main program for the ssh client.
 */
int
main(int ac, char **av)
{
	int i, r, opt, exit_status, use_syslog;
	char *p, *cp, *line, *argv0, buf[MAXPATHLEN];
	struct stat st;
	struct passwd *pw;
	int dummy, timeout_ms;
	extern int optind, optreset;
	extern char *optarg;
	struct servent *sp;
	Forward fwd;
	
	/* Ensure that fds 0, 1 and 2 are open or directed to /dev/null */
	sanitise_stdfd();
	
	__progname = ssh_get_progname(av[0]);
	init_rng();
	
		/*
	 * Save the original real uid.  It will be needed later (uid-swapping
	 * may clobber the real uid).
	 */
	original_real_uid = getuid();
	original_effective_uid = geteuid();

	/*
	 * Use uid-swapping to give up root privileges for the duration of
	 * option processing.  We will re-instantiate the rights when we are
	 * ready to create the privileged port, and will permanently drop
	 * them when the port has been created (actually, when the connection
	 * has been made, as we may need to create the port several times).
	 */
	PRIV_END;
	
	#ifdef HAVE_SETRLIMIT
	/* If we are installed setuid root be careful to not drop core. */
	if (original_real_uid != original_effective_uid) {
		struct rlimit rlim;
		rlim.rlim_cur = rlim.rlim_max = 0;
		if (setrlimit(RLIMIT_CORE, &rlim) < 0)
			fatal("setrlimit failed: %.100s", strerror(errno));
	}
#endif
	/* Get user data. */
	pw = getpwuid(original_real_uid);
	if (!pw) {
		logit("You don't exist, go away!");
		exit(255);
	}
	
	/* Take a copy of the returned structure. */
	pw = pwcopy(pw);
	
	/*
	 * Set our umask to something reasonable, as some files are created
	 * with the default umask.  This will make them world-readable but
	 * writable only by the owner, which is ok for all files for which we
	 * don't set the modes explicitly.
	 */
	umask(022);
	
	/*
	 * Initialize option structure to indicate that no values have been
	 * set.
	 */
	//initialize_options(&options);
	
	/* Parse command-line arguments. */
	host = NULL;
	use_syslog = 0;
	argv0 = av[0];
	
	 again:
	while ((opt = getopt(ac, av, "1246ab:c:e:fgi:kl:m:no:p:qstvx"
	    "ACD:F:I:KL:MNO:PR:S:TVw:XYy")) != -1) {
		switch (opt) {
		case '1':
			options.protocol = SSH_PROTO_1;
			break;
		case '2':
			options.protocol = SSH_PROTO_2;
			break;
		case '4':
			options.address_family = AF_INET;
			break;
			
		case '6':
			options.address_family = AF_INET6;
			break;
		case 'n':
			stdin_null_flag = 1;
			break;
		case 'f':
			fork_after_authentication_flag = 1;
			stdin_null_flag = 1;
			break;
		case 'x':
			options.forward_x11 = 0;
			break;
			
		case 'X':
			options.forward_x11 = 1;
			break;
		case 'y':
			use_syslog = 1;
			break;
		case 'Y':
			options.forward_x11 = 1;
			options.forward_x11_trusted = 1;
			break;
		case 'g':
			options.gateway_ports = 1;
			break;
			
		case 'O':
			if (strcmp(optarg, "check") == 0)
				muxclient_command = SSHMUX_COMMAND_ALIVE_CHECK;
			else if (strcmp(optarg, "exit") == 0)
				muxclient_command = SSHMUX_COMMAND_TERMINATE;
			else
				fatal("Invalid multiplex command.");
			break; 
		case 'P':	/* deprecated */
			options.use_privileged_port = 0;
			break;
		case 'a':
			options.forward_agent = 0;
			break;
		case 'A':
			options.forward_agent = 1;
			break;
			
		case 'k':
			options.gss_deleg_creds = 0;
			break;
		case 'K':
			options.gss_authentication = 1;
			options.gss_deleg_creds = 1;
			break;
		case 'i':
			if (stat(optarg, &st) < 0) {
				fprintf(stderr, "Warning: Identity file %s "
				    "not accessible: %s.\n", optarg,
				    strerror(errno));
				break;
			}
			if (options.num_identity_files >=
			    SSH_MAX_IDENTITY_FILES)
				fatal("Too many identity files specified "
				    "(max %d)", SSH_MAX_IDENTITY_FILES);
			options.identity_files[options.num_identity_files++] =
			    xstrdup(optarg);
			break;
			
		case 'I':
#ifdef SMARTCARD
			options.smartcard_device = xstrdup(optarg);
#else
			fprintf(stderr, "Warning: Identity file %s "
				    "not accessible: %s.\n");
#endif
		case 't':
			if (tty_flag)
				force_tty_flag = 1;
			tty_flag = 1;
			break;
		case 'v':
			if (debug_flag == 0) {
				debug_flag = 1;
				options.log_level = SYSLOG_LEVEL_DEBUG1;
			} else {
				if (options.log_level < SYSLOG_LEVEL_DEBUG3)
					options.log_level++;
				break;
			}
			/* FALLTHROUGH */
		case 'V':
			fprintf(stderr, "%s, %s\n",
			    SSH_RELEASE, SSLeay_version(SSLEAY_VERSION));
			if (opt == 'V')
				exit(0);
			break;
		case 'w':
			if (options.tun_open == -1)
				options.tun_open = SSH_TUNMODE_DEFAULT;
			options.tun_local = a2tun(optarg, &options.tun_remote);
			if (options.tun_local == SSH_TUNID_ERR) {
				fprintf(stderr,
				    "Bad tun device '%s'\n", optarg);
				exit(255);
			}
			break;
		case 'q':
			options.log_level = SYSLOG_LEVEL_QUIET;
			break;
		case 'e':
			if (optarg[0] == '^' && optarg[2] == 0 &&
			    (u_char) optarg[1] >= 64 &&
			    (u_char) optarg[1] < 128)
				options.escape_char = (u_char) optarg[1] & 31;
			else if (strlen(optarg) == 1)
				options.escape_char = (u_char) optarg[0];
			else if (strcmp(optarg, "none") == 0)
				options.escape_char = SSH_ESCAPECHAR_NONE;
			else {
				fprintf(stderr, "Bad escape character '%s'.\n",
				    optarg);
				exit(255);
			}
			break;
		case 'c':
			if (ciphers_valid(optarg)) {
				/* SSH2 only */
				options.ciphers = xstrdup(optarg);
				options.cipher = SSH_CIPHER_INVALID;
			} else {
				/* SSH1 only */
				options.cipher = cipher_number(optarg);
				if (options.cipher == -1) {
					fprintf(stderr,
					    "Unknown cipher type '%s'\n",
					    optarg);
					exit(255);
				}
				if (options.cipher == SSH_CIPHER_3DES)
					options.ciphers = "3des-cbc";
				else if (options.cipher == SSH_CIPHER_BLOWFISH)
					options.ciphers = "blowfish-cbc";
				else
					options.ciphers = (char *)-1;
			}
			break;
			
		case 'm':
			if (mac_valid(optarg))
				options.macs = xstrdup(optarg);
			else {
				fprintf(stderr, "Unknown mac type '%s'\n",
				    optarg);
				exit(255);
			}
			break;
		case 'M':
			if (options.control_master == SSHCTL_MASTER_YES)
				options.control_master = SSHCTL_MASTER_ASK;
			else
				options.control_master = SSHCTL_MASTER_YES;
			break;
		case 'p':
			options.port = a2port(optarg);
			if (options.port <= 0) {
				fprintf(stderr, "Bad port '%s'\n", optarg);
				exit(255);
			}
			break;
		case 'l':
			options.user = optarg;
			break;

		case 'L':
			if (parse_forward(&fwd, optarg, 0, 0))
				add_local_forward(&options, &fwd);
			else {
				fprintf(stderr,
				    "Bad local forwarding specification '%s'\n",
				    optarg);
				exit(255);
			}
			break;
			
		case 'R':
			if (parse_forward(&fwd, optarg, 0, 1)) {
				add_remote_forward(&options, &fwd);
			} else {
				fprintf(stderr,
				    "Bad remote forwarding specification "
				    "'%s'\n", optarg);
				exit(255);
			}
			break;
			
		case 'D':
			if (parse_forward(&fwd, optarg, 1, 0)) {
				add_local_forward(&options, &fwd);
			} else {
				fprintf(stderr,
				    "Bad dynamic forwarding specification "
				    "'%s'\n", optarg);
				exit(255);
			}
			break;
		case 'C':
			options.compression = 1;
			break;
		case 'N':
			no_shell_flag = 1;
			no_tty_flag = 1;
			break;
		case 'T':
			no_tty_flag = 1;
			break;
		case 'o':
			dummy = 1;
			line = xstrdup(optarg);
			if (process_config_line(&options, host ? host : "",
			    line, "command-line", 0, &dummy) != 0)
				exit(255);
			xfree(line);
			break;
				case 's':
			subsystem_flag = 1;
			break;
		case 'S':
			if (options.control_path != NULL)
				free(options.control_path);
			options.control_path = xstrdup(optarg);
			break;
		case 'b':
			options.bind_address = optarg;
			break;
		case 'F':
			config = optarg;
			break;
	


		default:
			usage();
		}
	}
	
	
	
}
