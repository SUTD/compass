
void foo(int x)
{
	int a;
	switch(x)
	{
		case 1:
			a = 1;
			break;
		case 2:
			a=2;
			break;
		case 3:
			a=3;
			break; 
		
		default:
			a=-1;
	}  

	if(x>=2 && x<=3) static_assert(a==x); 
	else static_assert(a == -1); //should fail
}