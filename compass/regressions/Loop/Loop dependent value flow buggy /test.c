//Loop/Loop dependent value flow buggy 
void foo(char* a, char* b, int size, int x, int y, int z)
{
	

	
	int i=0;
	x = 1;
	y=2;
	z=3;
	for(i=0; i<size; i++)
	{
		if(i%3==0) x=y;
		else y=z;
	}
	//should fail
	static_assert(x!=z);
	
	
}
