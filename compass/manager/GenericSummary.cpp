/*
 * GenericSummary.cpp
 *
 *  Created on: Aug 15, 2008
 *      Author: tdillig
 */

#include "GenericSummary.h"



GenericSummary::GenericSummary() {
	// TODO Auto-generated constructor stub
	data = "generic summary";
}

bool GenericSummary::operator==(const Summary& other)
{
	return this->data == ((GenericSummary &) other).data;
}



GenericSummary::~GenericSummary() {
	// TODO Auto-generated destructor stub
}
