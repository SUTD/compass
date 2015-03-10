/*
 * Cast.h
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 *
 *   v = (T) symbol;
 */

#ifndef SAIL_CAST_H_
#define SAIL_CAST_H_

#include "SaveInstruction.h"

#include <string>
using namespace std;

#include "type.h"

namespace il{class node;}
namespace sail {

class Variable;
class Symbol;

/**
 * \brief A cast instruction of the form v = (T) s
 */
class Cast:public SaveInstruction {
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & v;
        ar & s;
        ar & t;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & v;
        ar & s;
        ar & t;
        il::type::register_loaded_typeref(&t);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
	Variable* v;
	Symbol* s;
	il::type* t;

public:
	Cast(Variable* v, Symbol* s, il::type* t, il::node* original, int line);
	Cast(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);
	Symbol* get_rhs();
	void set_rhs(Symbol* s);

	/**
	 *
	 * @return the type that is being cast to.
	 */
	il::type* get_cast_type();
	virtual bool is_removable();
	virtual ~Cast();
};

}

#endif /* SAIL_CAST_H_ */
