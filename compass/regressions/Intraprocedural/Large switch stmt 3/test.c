// Intraprocedural/Large switch stmt 3

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
		case 13:
			a = 13;
			break;
		case 14:
			a=14;
			break;
		case 15:
			a=15;
			break; 
		case 16:
			a = 16;
			break;
		case 17:
			a=17;
			break;
		case 18:
			a=18;
			break; 
		case 19:
			a = 19;
			break;
		case 20:
			a=20;
			break;
		case 21:
			a=21;
			break;
		case 22:
			a=22;
			break; 
		case 23:
			a = 23;
			break;
		case 24:
			a=24;
			break;
		case 25:
			a=25;
			break; 
		case 26:
			a = 26;
			break;
		case 27:
			a=27;
			break;
		case 28:
			a=28;
			break; 
		case 29:
			a = 29;
			break;
		case 30:
			a=30;
			break;
		
		default:
			a=-1;
	}
	
	if(x==1 || x==2 || x==3)
		static_assert(a!=-1);

	if(x>=1 && x<=30) {
		static_assert(a!=-1);
		static_assert(x==a);
	}
	if(x>30)
		static_assert(a==-1);
}