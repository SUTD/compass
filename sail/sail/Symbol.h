/*
 * Symbol.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *  In SAIL, a symbol can be either a variable or a constant.
 */

#ifndef COMPASS_SYMBOL_H_
#define COMPASS_SYMBOL_H_

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/set.hpp>

#include <string>
using namespace std;

namespace il{
	class node;
	class type;
}

namespace sail {

/**
 * \brief Represents either a variable or a constant.
 */
class Symbol {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    }
public:
	Symbol();
	virtual string to_string() const = 0;
	virtual string to_string(bool print_original) const = 0;
	virtual il::node* get_original_node() = 0;
	virtual bool is_constant() = 0;
	virtual bool is_variable() = 0;
	virtual il::type* get_type() = 0;
	virtual ~Symbol();
	virtual bool is_temp();
};

}

#endif /* COMPASS_SYMBOL_H_ */
