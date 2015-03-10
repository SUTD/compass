/*
 * Type.h
 *
 *  Created on: Oct 16, 2008
 *      Author: tdillig
 */

#ifndef UTIL_TYPE_H_
#define UTIL_TYPE_H_

#include "il/type.h"

class Type {
public:
	static bool is_type_compatible(il::type* t1, il::type* t2, bool
			check_supertype_only = false);
	static bool is_supertype(il::type* t1, il::type* t2);

	static il::type* get_type_of_first_field(il::type* t);
};

#endif /* UTIL_TYPE_H_ */

