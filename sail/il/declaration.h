#ifndef DECLARATION_H_
#define DECLARATION_H_

#include "node.h"
#include <assert.h>

namespace il
{
/**
 * \brief An abstract class representing variable and function decalarations
 */
class declaration : public il::node
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::node>(*this);
    }

public:
	declaration();
	virtual ~declaration();
	virtual bool is_fun_decl() = 0;
	virtual bool is_var_decl() = 0;
	virtual void print()=0;
};

}

#endif /*DECLARATION_H_*/
