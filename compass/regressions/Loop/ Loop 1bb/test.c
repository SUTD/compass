// Loop/ Loop 1bb

void foo(int size, int* a)
{
	int i;
	for(i=0; i<size; i+=3)
	{

	}
	//should fail
	static_assert(i>size);
	static_assert(i%3==0);
	

}
