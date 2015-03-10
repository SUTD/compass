// Loop/Loop 2d

void foo(int* a, int size, int flag)
{
	int i;
	for(i=0; i<size; i++)
	{
		if(flag) a[i] = 0;
		else a[i] = 1;
	}
	if(size >=3 && flag) {
		static_assert(a[0] ==0);
		static_assert(a[1] == 0); 
		static_assert(a[2]==0);
	}

	else if(size >= 3){
		static_assert(a[0] ==1);
		static_assert(a[1] == 1); 
		static_assert(a[2]==1);
	}

}
