#ifndef CONTROL_STATEMENT_H_
#define CONTROL_STATEMENT_H_

#include "statement.h"
#include <assert.h>

namespace il
{

/**
 * \brief An abstract class representing any control statement
 */
class control_statement : public il::statement
{

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::statement>(*this);
    }

public:
	control_statement();
	virtual string to_string() const = 0;
	virtual ~control_statement();
};

}

#endif /*CONTROL_STATEMENT_H_*/
