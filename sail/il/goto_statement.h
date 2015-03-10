#ifndef GOTO_STATEMENT_H_
#define GOTO_STATEMENT_H_

#include "control_statement.h"


namespace il
{

class label;

/**
 * \brief A goto statement
 */
class goto_statement : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
    	ar & label_name;
    	ar & l;
    }
private:
	string label_name;
	label *l;
public:
	goto_statement();
	goto_statement(label *l, string label_name, location loc);
	string get_label_name();
	void print();
	virtual ~goto_statement();
	label* get_label();
	label** get_label_ref();
	virtual string to_string() const;
};

}

#endif /*GOTO_STATEMENT_H_*/
