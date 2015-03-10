#ifndef BREAK_STATEMENT_H_
#define BREAK_STATEMENT_H_

#include "control_statement.h"
namespace il
{

/**
 * \brief Represents a break construct
 */
class break_statement : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
    }


public:
	break_statement();
	void print();
	virtual ~break_statement();
	virtual string to_string() const;

};

}

#endif /*BREAK_STATEMENT_H_*/
