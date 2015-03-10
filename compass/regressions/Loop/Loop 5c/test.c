// Loop/Loop 5c

void foo(int* a, int size)
{
	int i;
	
	for(i=0; i<size; i++)
	{
		a[i] = i;
	}

if(size >=3) {
	static_assert(a[0] ==0);
	static_assert(a[1] == 1); 
	static_assert(a[2]==2);
}
	

}