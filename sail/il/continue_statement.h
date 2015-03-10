#ifndef CONTINUE_STATEMENT_H_
#define CONTINUE_STATEMENT_H_

#include "control_statement.h"

namespace il
{

/**
 * \brief A continue construct inside a loop
 */
/**
 * Since GCC replaces continue's with goto's,
 * continue statements currently do not appear in SAIL.
 */
class continue_statement : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
    }

public:
	continue_statement();
	void print();
	virtual string to_string() const;
	virtual ~continue_statement();
};

}

#endif /*CONTINUE_STATEMENT_H_*/
