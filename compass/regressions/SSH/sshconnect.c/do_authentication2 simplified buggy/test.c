#define NULL 0

struct s {
	char	*name;
	int	(*userauth)();
};

typedef struct s s;




int userauth_pubkey()
{
  return 2;
}

int userauth_none()
{
  return 2;
}




s method_pubkey = {
	"publickey",
	userauth_pubkey,
};

s method_none = {
	"none",
	NULL, // mistake!
};






static s *
lookup(struct s** array, int* x, int num)
{
	int i;

      for (i = 0; array[i] != NULL; i++)
	     if(foo())
		      return array[i];

	return NULL;
}



char* get_method();

/*ARGSUSED*/
static s*
input_userauth_request(s** array, int* x, int size)
{


	s *m = NULL;
	char *method = get_method();


	/* try to authenticate user */
	m = lookup(array,  x, size);

	if (m != NULL) {
	    static_assert(m->userauth != NULL); //should fail
	}
	

      return m;

}



void
test(int* x)
{
   s *array[] = {
	&method_pubkey,
	&method_none, // userauth field of method_none can be NULL!
	NULL
    };

	s* m = input_userauth_request(array, x, 3);



}