// SSH/hostfile.c host_hash

#include<stdlib.h>

#define SHA_DIGEST_LENGTH 1024

typedef unsigned int u_int;

#define HASH_MAGIC	"|1|"
#define HASH_DELIM	'|'

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