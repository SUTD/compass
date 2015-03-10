// Interprocedural/Call 8
#include <stdlib.h>

struct s{
	int x;
	int y;
};

struct s* bar(struct s* my_s)
{
	struct s* new_s = malloc(sizeof(struct s));
	*new_s = *my_s;
	my_s->x = 2;
	my_s->y = 3;
	return new_s;
}
