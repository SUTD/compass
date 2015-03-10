/*
 * Identifier.cpp
 *
 *  Created on: Feb 16, 2010
 *      Author: tdillig
 */

#include "Identifier.h"
#include "util.h"
#include "type.h"
#include <assert.h>

#define NOT_LOOP -1

Identifier::Identifier(const FileIdentifier& dir, const string & function_name,
		const il::namespace_context & ns,
		il::function_type* signature, language_type language): dir(dir), ns(ns)
{
	cout << "new identifer: " << function_name << " dir: " << dir.to_string('/')
			<< endl;
	this->function_name = function_name;
	assert(signature != NULL);
	this->signature = signature;
	this->loop_id = NOT_LOOP;
	this->language = language;
}

Identifier::Identifier(const Identifier& fun_id,
		int loop_id):dir(fun_id.dir)
{
	this->function_name = fun_id.function_name;
	this->ns = fun_id.ns;
	this->signature = fun_id.signature;
	this->loop_id = loop_id;
	this->language = fun_id.language;
}

Identifier::Identifier(const Identifier& other):dir(other.dir)
{
	this->function_name = other.function_name;
	this->ns = other.ns;
	this->signature = other.signature;
	this->loop_id = other.loop_id;
	this->language = other.language;
}

string Identifier::to_string(char dir_separator) const
{
	if(signature == NULL) return "<empty>";
	string res;

	if(language != LANG_JAVA)
	{
	    res = dir.to_string(dir_separator);
	    res += dir_separator;
	}
    res += ns.to_string();

	res+= function_name;
	res+= "<" + signature->to_string() + ">";
	if(loop_id != NOT_LOOP) res += "@" + int_to_string(loop_id);
	return res;

}
string Identifier::to_short_string() const
{
	if(signature == NULL) return "<empty>";
	string res = function_name;
	if(language == LANG_JAVA) {
		size_t i = res.rfind('/');
		if(i != string::npos){
			res.substr(i+1);
		}
	}
	return res;

}

const string& Identifier::get_function_name() const
{
	return this->function_name;
}
const il::namespace_context& Identifier::get_namespace() const
{
	return ns;
}

il::function_type* Identifier::get_function_signature() const
{
	return this->signature;
}
const FileIdentifier& Identifier::get_file_id() const
{
	return this->dir;
}
bool Identifier::is_loop_id() const
{
	return this->loop_id  != NOT_LOOP;
}
bool Identifier::is_function_id() const
{
	return this->loop_id  == NOT_LOOP;
}
int Identifier::get_loop_id() const
{
	return this->loop_id;
}

language_type Identifier::get_language() const
{
	return language;
}

bool Identifier::is_empty() const
{
	return loop_id == 0 && signature == NULL && function_name == "";
}

Identifier Identifier::get_function_identifier() const
{
	if(loop_id == NOT_LOOP) return *this;
	return Identifier(dir, function_name, ns, signature, language);
}
bool Identifier::operator==(const Identifier & other) const
{
	if(this->dir != other.dir) return false;
	if(this->function_name != other.function_name) return false;
	if(this->ns != other.ns) return false;
	if(this->loop_id != other.loop_id) return false;
	if(this->language != other.language) return false;
	if(this->signature == NULL && other.signature == NULL) return true;
	if(this->signature == NULL || other.signature == NULL) return false;
	return *this->signature == *other.signature;
}
bool Identifier::operator<(const Identifier & other) const
{
	if(this->dir > other.dir) return false;
	if(this->dir < other.dir) return true;
	if(this->function_name > other.function_name) return false;
	if(this->function_name < other.function_name) return true;
	if(this->ns > other.ns) return false;
	if(this->ns < other.ns) return true;
	if(this->loop_id > other.loop_id) return false;
	if(this->loop_id < other.loop_id) return true;
	if(this->language > other.language) return false;
	if(this->language < other.language) return true;
	if(this->signature == NULL && other.signature == NULL) return false;
	if(this->signature == NULL) return true;
	if(other.signature == NULL) return false;
	return this->signature->to_string() < other.signature->to_string();
}



Identifier::~Identifier()
{

}


ostream& operator <<(ostream &os, const Identifier &obj)
{
	 os  << obj.to_string();
	 return os;
}

