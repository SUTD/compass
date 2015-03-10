//Loop/Strings 1 buggy
void foo(char* a, char* b, int size, int x, int y, int z, int k)
{
	assume(k>=0 && k<buffer_size(b)-1);
	a = b+k;
	while(a > b){
	      if( *a != '/') break;
		 a--;
	}
	//should fail
	buffer_safe(a, 2);
	
}

