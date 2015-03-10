#ifndef STRING_CONST_EXP_H_
#define STRING_CONST_EXP_H_

#include "const_expression.h"

namespace il
{
/**
 * \brief A string constant
 */
class string_const_exp : public il::const_expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::const_expression>(*this);
    	ar & str_const;
    }
private:
	string str_const;

public:
	string_const_exp();
	string_const_exp(string str_const, type* t, location loc);
	virtual ~string_const_exp();
	virtual string to_string() const;
	string get_string();
};

}

#endif /*STRING_CONST_EXP_H_*/
