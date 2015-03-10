#ifndef INTEGER_CONST_EXP_H_
#define INTEGER_CONST_EXP_H_

#include "const_expression.h"

namespace il
{

/**
 * \brief Representation of an integer constant
 */
class integer_const_exp : public il::const_expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::const_expression>(*this);
    	ar & int_const;
    	ar & is_signed;
    	ar & is_char;
    }
private:
	long int  int_const;
	bool is_signed;
	bool is_char;
public:
	integer_const_exp();
	integer_const_exp(long int int_const, type* t, location loc);
	virtual string to_string() const;
	long int get_integer();
	bool isSigned();
	bool isChar();
	virtual ~integer_const_exp();
};

}

#endif /*INTEGER_CONST_EXP_H_*/
