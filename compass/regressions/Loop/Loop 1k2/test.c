// Loop/Loop 1k2

void foo(int size, int* a, int flag)
{
	int i;
	for(i=1; i<size;)
	{
		if(flag) i++;
		else i+=3;

	}
	static_assert(i>=size);
	// should fail
	if(!flag) static_assert(i%3==0);

}

