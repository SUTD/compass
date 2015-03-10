//  Intraprocedural/Pointer arithmetic 1

void foo(int x)
{
	int b[10];
	int* t= &b[2];
	* t= 77;
	int y = b[2];
	static_assert(y==77);

	t++;
	*t = 44;
	static_assert(b[3]==44);

	t[2] = 33;
	static_assert(b[5] == 33);

	t[3] = x;
	static_assert(b[6] == x); 
	
}

