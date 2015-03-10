// Intraprocedural/Stmt guard 12

void foo(int flag, int a)
{
	int b = 0;
	if(flag) {
		if(a>5) {
			b=1;
		}
	}

	if(flag && a>5) static_assert(b==1);
	else static_assert(b==0);
}