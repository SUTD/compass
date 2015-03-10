#include "expression.h"

namespace il
{

expression::expression()
{
}

expression::~expression()
{
}


ostream& operator <<(ostream &os, const expression &obj)
{
      os << obj.to_string();
      return os;
}


}


