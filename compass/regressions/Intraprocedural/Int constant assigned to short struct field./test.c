struct dd
{
  short x;
  short y;
};


void foo(struct dd* d)
{
   d->x = 2;
   static_assert(d->x == 2);
}