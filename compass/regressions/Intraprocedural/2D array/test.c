// Intraprocedural/2D array

void foo(int x, int i, int j)
{
	int a[2][3];
	a[0][0] =  1;
	a[0][1] = 2;
	a[0][2] = 3;
	a[1][0]= 4;
	a[1][1]= 5;
	a[1][2] = 6;
	static_assert(a[0][0] == 1);
	static_assert(a[0][1] == 2);
	static_assert(a[0][2] == 3);
	static_assert(a[1][0] == 4);
	static_assert(a[1][1] == 5);
	static_assert(a[1][2] == 6);

	if(i>=0 && i<=1 && j>=0 && j<=2)
		static_assert(a[i][j] == 3*i+j+1);
	
}
