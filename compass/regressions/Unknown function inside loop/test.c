// Unknown function inside loop


void foo()
{
	int i;
	int a[10];
	for(i=0; i<10; i++)
	{
		a[i] = unknown();
	}
	// should fail
	static_assert(a[0] == a[1]); 
}
