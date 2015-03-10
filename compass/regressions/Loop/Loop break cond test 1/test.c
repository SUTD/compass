// Loop/Loop break cond test 1

void foo(int x, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		// should fail
		static_assert(x<=3);
		if(x>3) break;
	}
}
