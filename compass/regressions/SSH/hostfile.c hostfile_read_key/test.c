// SSH/hostfile.c hostfile_read_key

void specify_checks()
{
	check_null();
	check_buffer();
}

#include<stdlib.h>

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

struct Key {
	int	 type;
	int	 flags;
	RSA	*rsa;
	DSA	*dsa;
};

typedef struct Key Key;

#define SHA_DIGEST_LENGTH 1024

typedef unsigned int u_int;

#define HASH_MAGIC	"|1|"
#define HASH_DELIM	'|'

typedef int EVP_MD;
typedef int HMAC_CTX;
/*
 * Parses an RSA (number of bits, e, n) or DSA key from a string.  Moves the
 * pointer over the key.  Skips any whitespace at the beginning and at end.
 */

int
hostfile_read_key(char **cpp, u_int *bitsp, Key *ret)
{
	char *cp;
	assume(cpp != NULL);
	assume(*cpp != NULL);
	assume(bitsp!=NULL);
	

	/* Skip leading whitespace. */
	for (cp = *cpp; *cp == ' ' || *cp == '\t'; cp++)
		;

	if (key_read(ret, &cp) != 1)
		return 0;
		

	/* Skip trailing whitespace. */
	for (; *cp == ' ' || *cp == '\t'; cp++)
		;

	/* Return results. */
	*cpp = cp;
	*bitsp = key_size(ret);
	return 1;
}
