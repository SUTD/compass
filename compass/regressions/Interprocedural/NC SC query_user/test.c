// Intraprocedural/NC &SC query_user

#include <stdlib.h>

int query_user(int feature_enabled)
{
	if(!feature_enabled) return 0;
	char x = get_user_input();
	if(x=='y') return 1;
	return 0;
}

void foo()
{
 	int res = query_user(0);
	static_assert(res == 0);

}