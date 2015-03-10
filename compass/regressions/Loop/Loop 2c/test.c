// Loop/Loop 2c

void foo(int flag)
{
	int i;
	int a[3];
	for(i=0; i<3; i++)
	{
		if(flag) a[i] = 0;
		else a[i] = 1;
	}
	if(flag) {
		static_assert(a[0] ==0);
		static_assert(a[1] == 0); 
		static_assert(a[2]==0);
	}

	else {
		static_assert(a[0] ==1);
		static_assert(a[1] == 1); 
		static_assert(a[2]==1);
	}

}
