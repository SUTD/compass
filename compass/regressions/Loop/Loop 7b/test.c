// Loop/Loop 7b


void foo(int* a, int size, int elem)
{
	int i;
	int pos = -1;
	for(i=0; i<size; i++)
	{
		if(a[i] == elem) 
		{
			pos = i;
			break;
		}
	}
	if(pos!= -1) static_assert(a[pos] == elem);	


}
