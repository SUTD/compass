// Interprocedural/Imprecision test 3

#include <stdlib.h>

int optind;
char* optarg;



int
getopt_long (int argc, char **argv)
{
      if(foo()) return *argv[optind];
   return -1;
  
}

void
usage (int status)
{

   exit (status);
}

int
main (int argc, char **argv)
{
          optind = 0;
   int x;
      if (getopt_long (argc, argv) != -1){
       usage (EXIT_FAILURE);
   }
  
   if (argc <= optind)  // COND 1
   {
       usage (EXIT_FAILURE);
   }
      static_assert(argc > optind);

}
