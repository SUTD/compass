// Loop/Last iteration Info Imprecision 

void foo(int size, int k)
{
	int i;
	for(i=0; i<size; i++)
	{
		if(faz()) break;
	}
	//should fail
	if(size > 0) static_assert(i==size || i==0);
}
