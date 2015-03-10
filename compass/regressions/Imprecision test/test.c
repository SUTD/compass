// Imprecision test

#include "stdlib.h"

 

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


/* key is stored in external hardware */
#define KEY_FLAG_EXT		0x0001

struct Key {
	int	 type;
	int	 flags;
	RSA	*rsa;
	DSA	*dsa;
};




// ------Unknown function decls---------
void
fatal(const char *fmt,...){exit(1);}
void* calloc(size_t, size_t);
RSA* RSA_new();
BIGNUM* BN_new();
DSA* DSA_new();

int x;
char *bar (__const char *s, int c)
{
	if(foo()) return foo2();
	return foo3();
}

// -------------------------------------


/* returns 1 ok, -1 error */
int
key_read(Key *ret, char **cpp)
{
	Key *k;
	int success = -1;
	char *cp, *space;



	switch (ret->type) {
	case KEY_RSA1:
		success = 1;
		break;
	case KEY_DSA:
		space = bar(cp, ' ');
		if (space == NULL) { 
			return -1;
		}
		
		
		break; 
	}
	return success;
}

void key_read_test(Key *ret, char **cpp)
{
	int res = key_read(ret, cpp);
	// should fail
	static_assert(res == -1);
	
}
