// Loop/Loop 7c2


void foo(int* a, int size, int elem, int flag)
{
	int i;
	int pos = -1;
	for(i=0; i<size; i++)
	{
		if(a[i] == elem && flag) 
		{
			pos = i;
			break;
		}
	}
	// should fail
	static_assert(a[pos] == elem);
	static_assert(flag!=0);
	


}
