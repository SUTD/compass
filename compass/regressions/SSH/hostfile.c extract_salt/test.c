// SSH/hostfile.c extract_salt

void specify_checks()
{
	check_null();
	check_buffer();
}

#include<stdlib.h>

#define SHA_DIGEST_LENGTH 1024

typedef unsigned int u_int;

#define HASH_MAGIC	"|1|"
#define HASH_DELIM	'|'

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

static int
extract_salt(const char *s, u_int l, char *salt, size_t salt_len)
{
	char *p, *b64salt;
	u_int b64len;
	int ret;

	if (l < sizeof(HASH_MAGIC) - 1) {
		debug2("extract_salt: string too short");
		return (-1);
	}
	if (strncmp(s, HASH_MAGIC, sizeof(HASH_MAGIC) - 1) != 0) {
		debug2("extract_salt: invalid magic identifier");
		return (-1);
	}
	s += sizeof(HASH_MAGIC) - 1;
	l -= sizeof(HASH_MAGIC) - 1;
	if ((p = memchr(s, HASH_DELIM, l)) == NULL) {
		debug2("extract_salt: missing salt termination character");
		return (-1);
	}

	b64len = p - s;
	/* Sanity check */
	if (b64len == 0 || b64len > 1024) {
		debug2("extract_salt: bad encoded salt length %u", b64len);
		return (-1);
	}
	b64salt = xmalloc(1 + b64len);
	memcpy(b64salt, s, b64len);
	b64salt[b64len] = '\0';

	ret = __b64_pton(b64salt, salt, salt_len);
	xfree(b64salt);
	if (ret == -1) {
		debug2("extract_salt: salt decode error");
		return (-1);
	}
	if (ret != SHA_DIGEST_LENGTH) {
		debug2("extract_salt: expected salt len %d, got %d",
		    SHA_DIGEST_LENGTH, ret);
		return (-1);
	}

	return (0);
}



typedef int EVP_MD;
typedef int HMAC_CTX;

char *
host_hash(const char *host, const char *name_from_hostfile, u_int src_len)
{
	const EVP_MD *md = EVP_sha1();
	HMAC_CTX mac_ctx;
	char salt[256], result[256], uu_salt[512], uu_result[512];
	static char encoded[1024];
	u_int i, len;

	len = EVP_MD_size(md);
	assume(len <= 256);

	if (name_from_hostfile == NULL) {
		/* Create new salt */
		for (i = 0; i < len; i++)
			salt[i] = arc4random();
	} else {
		/* Extract salt from known host entry */
		if (extract_salt(name_from_hostfile, src_len, salt,
		    sizeof(salt)) == -1)
			return (NULL);
	}

	HMAC_Init(&mac_ctx, salt, len, md);
	HMAC_Update(&mac_ctx, host, strlen(host));
	HMAC_Final(&mac_ctx, result, NULL);
	HMAC_cleanup(&mac_ctx);

	if (__b64_ntop(salt, len, uu_salt, sizeof(uu_salt)) == -1 ||
	    __b64_ntop(result, len, uu_result, sizeof(uu_result)) == -1)
		fatal("host_hash: __b64_ntop failed");

	snprintf(encoded, sizeof(encoded), "%s%s%c%s", HASH_MAGIC, uu_salt,
	    HASH_DELIM, uu_result);

	return (encoded);
}
