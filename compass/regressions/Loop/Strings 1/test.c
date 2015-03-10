//Loop/Strings 1
void foo(char* a, char* b, int size, int x, int y, int z, int k)
{
	assume(k>=0 && k<buffer_size(b)-1);
	a = b+k;
	while(a > b){
	      if( *a != '/') break;
		 a--;
	}
	buffer_safe(a, 1);
	
}
