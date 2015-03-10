/*
 * call_id.cpp
 *
 *  Created on: Apr 12, 2010
 *      Author: tdillig
 */

#include "call_id.h"
#include "Identifier.h"
#include "il/type.h"
#include "util.h"

call_id::call_id(const Identifier& id)
{
	this->name = id.get_function_name();
	this->ns = id.get_namespace();
	this->sig = id.get_function_signature();
	this->loop_id = id.get_loop_id();
	this->language = id.get_language();

}

call_id::call_id()
{
	this->sig = NULL;
	this->loop_id = -1;
	this->language = LANG_UNSUPPORTED;
}


call_id::call_id(const call_id & other)
{
	this->name = other.name;
	this->ns = other.ns;
	this->sig = other.sig;
	this->loop_id = other.loop_id;
	this->language = other.language;
}
call_id::call_id(const string & name, const il::namespace_context& ns,
		il::type* sig, language_type language)
{
	this->name = name;
	this->ns = ns;
	this->sig = sig;
	this->loop_id = -1;
	this->language = language;
}

call_id::call_id(int loop_id, const string & name,
		const il::namespace_context& ns, il::type* sig, language_type language)
{
	this->name = name;
	this->ns = ns;
	this->sig = sig;
	this->loop_id = loop_id;
	this->language = language;
}

const il::namespace_context& call_id::get_namespace()
{
	return ns;
}

const string& call_id::get_function_name()
{
	return name;
}
il::type* call_id::get_signature() const
{
	return sig;
}


bool call_id::operator==(const call_id & other) const
{
	if(this->name != other.name) return false;
	if(ns != other.ns) return false;
	if(loop_id != other.loop_id) return false;

	if(language == LANG_C || other.language == LANG_C) return true;
	return this->sig == other.sig;
}
bool call_id::operator<(const call_id& other) const
{
	if(this->name > other.name) return false;
	if(this->name < other.name) return true;
	if(this->ns > other.ns) return false;
	if(ns < other.ns) return true;
	if(this->loop_id > other.loop_id) return false;
	if(this->loop_id < other.loop_id) return true;

	if(language == LANG_C || other.language == LANG_C) return false;
	return this->sig < other.sig;
}
string call_id::to_string() const
{
	string res = ns.to_string();
	res += name + "<" + sig->to_string() + ">";
	if(loop_id != -1)
		res += ":" + int_to_string(loop_id);
	return res;
}
