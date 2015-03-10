// Loop/ Loop 2ff

void foo(int size, int* a)
{
	int i;
	for(i=0; i<size; i++)
	{
		a[i] = 0;
	}
	int k;
	if(k>=0 && k<size)
		static_assert(a[k] == 0);


}
