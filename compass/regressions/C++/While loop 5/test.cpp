
int rand();

void foo()
{
  int k=1;
  while(1) {
    while(1) {if(rand()) break;}
    static_assert(0); //should fail
    if(rand()) break;
  } 
  


}