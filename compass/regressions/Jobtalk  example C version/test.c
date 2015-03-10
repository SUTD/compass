// Intraprocedural/test assume size

#include <stdlib.h>


struct Data
{
  int x;

};

struct Packet {
  struct Data* val;
};




void foo(struct Data** data, int size)
{

  assume_size(data, size);
  assume(data!=NULL);
  assume(size > 0);

  struct Packet** packets = malloc(size * sizeof(struct Packet*));

  
  int i = size-1;
  int j = 0;
  for(; j<size; j++, i--) {
    struct Packet* cur = malloc(sizeof(struct Packet)); 
    cur->val = data[j];
    packets[i] = cur;
  }

 struct Packet* p = packets[0];
 struct Data* d = p-> val;
 struct Data* last = data[size-1];
 static_assert(d == last);
  
}


  