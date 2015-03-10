// Intraprocedural/Large switch stmt 1

void foo()
{
	int a;
	int x;
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
		case 4:
			a = 4;
			break;
		case 5:
			a=5;
			break;
		case 6:
			a=6;
			break; 
		case 7:
			a = 7;
			break;
		case 8:
			a=8;
			break;
		case 9:
			a=9;
			break; 
		case 10:
			a = 10;
			break;
		case 11:
			a=11;
			break;
		case 12:
			a=12;
			break; 
		
		default:
			a=-1;
	}
	
	if(x==1 || x==2 || x==3)
		static_assert(a!=-1);

	if(x>=1 && x<=12) {
		static_assert(a!=-1);
		static_assert(x==a);
	}
	if(x>12)
		static_assert(a==-1);
}