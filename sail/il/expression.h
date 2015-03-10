#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include "node.h"
#include <assert.h>

namespace il
{
class type;

/**
 * \brief An abstract class that all different expressions inherit from
 */
class expression : public il::node
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<il::node>(*this);
        ar & t;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<il::node>(*this);
        ar & t;
        type::register_loaded_typeref(&t);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

protected:
	type* t;

public:
	expression();
	virtual ~expression();
	virtual string to_string() const = 0;
	virtual type* get_type(){return t;};
	void print(){};
	friend ostream& operator <<(ostream &os, const expression &obj);
};

}

#endif /*EXPRESSION_H_*/
