using namespace spec;


struct Data
{
  int x;

};

struct Packet {
  Data* val;
};

void foo(vector<Data*>& _data)
{


  vector<Data*>& data = _data;

  if(data.size() == 0) return;
  int x = data.size();


  list<Packet*> packets;

  for(int j=0; j<x; j++) {
    Packet* cur = new Packet();
    cur->val = data[j];
    packets.push_front(cur);
  }
  Packet* p = packets.back();
  Data* d = p->val;
  static_assert(data[0] == d);


  
}