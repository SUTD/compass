/*
 * Label.cpp
 *
 *  Created on: July 28, 2012
 *      Author: tdillig
 */

#include "node.h"
#include <assert.h>
#include "ExceptionReturn.h"

namespace sail {




string ExceptionReturn::to_string() const
{
	return "exception_return()";
}

string ExceptionReturn::to_string(bool pretty_print) const
{
	return "exception_return()";
}


ExceptionReturn::ExceptionReturn(){
	this->inst_id = EXCEPTION_RETURN;
	//haiyan added
	this->original = NULL;
}

bool ExceptionReturn::is_removable()
{
	return false;
}


ExceptionReturn::~ExceptionReturn() {

}

}
