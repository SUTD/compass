#ifndef CASE_LABEL_H_
#define CASE_LABEL_H_

#include "control_statement.h"


namespace il
{

/**
 * \brief A case label
 */
/**
 * In case labels, it's legal to specify a range of values
 * using the syntax "case 3 ... 7". Hence, in our representation,
 * a case label has a lower and a higher value to represent this range.
 * For a constant such as "case 'a'", the low and high values
 * will be the same.
 */

class case_label : public il::control_statement
{

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
        ar & low;
        ar & high;
        ar & is_default;
    }

private:

	long low;
	long high;
	bool is_default;



public:
	case_label();
	case_label(long low, long high, location loc);
	case_label(long num, location loc);
	case_label(location loc);
	virtual string to_string() const;
	virtual void print(){};

	/**
	 * Is this a range such as case 3 ... 7 or
	 * just a constant like case 3?
	 */
	bool has_range() const;

	/**
	 * Is this the "default" label in a switch statement?
	 */
	bool is_default_label() const;

	/**
	 * @return Low value in the range case x ... y
	 */
	long get_low();

	/**
	 * @return High value in the range case x ... y
	 */
	long get_high();
	virtual ~case_label();
};

}

#endif /*CASE_LABEL_H_*/
