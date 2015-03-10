// buffer access/strchr 1
#include <stdlib.h>

void specify_checks()
{
	check_null();
	check_buffer();
}


int x;
char *strchr (const char *s, int c)
{
	assume(x>=0 && x<buffer_size(s));
	if(rand()) return &s[x];
	return NULL;
}


#include <stdlib.h>

#define MAX_SEND_ENV 36

struct Options
{
  int num_send_env;
  char* send_env[MAX_SEND_ENV];
};


struct Options options;
unsigned int env_len;
void
client_session2_setup(int id, int want_tty, int want_subsystem,
    const char *term, char **env)
{
	int len;
	int c;

	assume(options.num_send_env <= MAX_SEND_ENV);
	assume(options.num_send_env >=0);
	assume(env_len*sizeof(char*) <= buffer_size(env));




	/* Transfer any environment variables from client to server */
	if (options.num_send_env != 0 && env != NULL) {
		int i, j, matched;
		char *name, *val;

		debug("Sending environment.");
		for (i = 0; i<env_len; i++) {
			/* Split */
			name = env[i];
			if ((val = strchr(name, '=')) == NULL) {
				continue;
			}
			int t = 5;

			matched = 0;
			for (j = 0; j < options.num_send_env; j++) {
				if (match_pattern(name, options.send_env[j])) {
					matched = 1;
					break;
				}
			}
		}
	}

}
