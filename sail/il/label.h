#ifndef LABEL_H_
#define LABEL_H_

#include "control_statement.h"

#include <string>

using namespace std;
namespace il
{

/**
 * \brief A label
 */
class label : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
    	ar & label_name;
    }
private:
	string label_name;
public:
	label();
	label(string label_name, location loc);
	string get_label();
	virtual ~label();
	virtual string to_string() const;
	void print();
};

}

#endif /*LABEL_H_*/
