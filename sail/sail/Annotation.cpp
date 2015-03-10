/*
 * Annotation.cpp
 *
 *  Created on: Jan 16, 2013
 *      Author: tdillig
 */

#include "Annotation.h"

namespace sail {

Annotation::Annotation() {


}

Annotation::~Annotation() {

}

FlowAnnotation::FlowAnnotation(const string & to, const string & from,
		const string & through): to(to), from(from), through(through)
{

}

const string & FlowAnnotation::get_to() const
{
	return to;
}
const string & FlowAnnotation::get_from() const
{
	return from;
}
const string & FlowAnnotation::get_through() const
{
	return through;
}
string FlowAnnotation::to_string() const
{
	return "@Flow(from=" + from + ",to="+to+",though=" + through + ")";
}

} /* namespace sail */
