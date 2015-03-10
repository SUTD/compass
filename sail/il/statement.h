#ifndef STATEMENT_H_
#define STATEMENT_H_

#include "node.h"
#include <assert.h>

namespace il
{

class statement : public il::node
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::node>(*this);
    }
public:
	friend ostream& operator <<(ostream &os, const statement *obj);
	virtual string to_string() const = 0;
	statement();
	virtual void assert_statement();
	virtual ~statement();
};

}

#endif /*STATEMENT_H_*/
