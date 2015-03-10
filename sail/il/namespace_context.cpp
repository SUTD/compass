

#include "namespace_context.h"
#include "serialization-hooks.h"
#include "util.h"


namespace il {


bool namespace_context::_init = namespace_context::init();

bool namespace_context::init()
{
	register_end_callback(_namespace_context::delete_unused_instances);
	return true;
}

namespace_context::namespace_context()
{
	this->ns = _namespace_context::make("", NULL);
}


namespace_context::namespace_context(const namespace_context & other)
{
	this->ns = other.ns;
}
namespace_context::namespace_context(const string &name,
		const namespace_context &context)
{
	ns = _namespace_context::make(name, context.ns);
}
namespace_context::namespace_context(const string &name)
{
	ns = _namespace_context::make(name, NULL);
}
string namespace_context::to_string() const
{
	return ns->to_string();
}
bool namespace_context::operator==(const namespace_context &other) const
{
	return ns == other.ns;
}

bool namespace_context::operator!=(const namespace_context& other) const
{
	return ns != other.ns;
}
bool namespace_context::operator<(const namespace_context& other) const
{
	return ns < other.ns;
}
bool namespace_context::operator>(const namespace_context& other) const
{
	return ns > other.ns;
}

const string& namespace_context::get_name()
{
	return ns->name;
}
namespace_context namespace_context::get_context()
{
	return namespace_context(ns->context);
}

namespace_context::namespace_context(_namespace_context* ns)
{
	if(ns == NULL) this->ns = _namespace_context::make("", NULL);
	else this->ns = ns;
}

size_t namespace_context::get_hash_code()
{
	return ns->hash_c;
}

/*
 *
 * **************************************************
 * INTERNAL
 * **************************************************
 *
 */

unordered_set<_namespace_context *, _namespace_context::nc_hash,
			_namespace_context::nc_eq> _namespace_context::instances;

unordered_set<_namespace_context*> _namespace_context::unused_instances;

_namespace_context::_namespace_context(
		const string &aname, _namespace_context *acontext)
    : name(aname), context(acontext)
{
		if(name == "") assert(context == NULL);
		if(context != NULL && context->name == "") {
			context = NULL;
		}

		compute_hash();
}

_namespace_context::_namespace_context()
{
	name = "";
}

_namespace_context::~_namespace_context()
{
	//cout << "Deleting ns:" << to_string() << " addr: " << this << endl;
}



_namespace_context* _namespace_context::get_instance(_namespace_context* ns)
{
	if(ns == NULL) return ns;
    unordered_set<_namespace_context *>::iterator it = instances.find(ns);
    if (it != instances.end()) {
    	if(*it == ns) return ns;
       // if(!in_serialization()) delete ns;
        else _namespace_context::unused_instances.insert(ns);
        return *it;
    }
    instances.insert(ns);
    return ns;
}

void _namespace_context::delete_unused_instances()
{
	/*unordered_set<_namespace_context*>::iterator it = unused_instances.begin();
	for(; it!= unused_instances.end(); it++)
	{
		_namespace_context* ns = *it;
		unordered_set<_namespace_context *>::iterator it = instances.find(ns);
		if(it != instances.end() && *it == ns) assert(false);
		 delete ns;
	}

	unused_instances.clear();
	*/
}

bool _namespace_context::is_empty()
{
	return name == "" && context == NULL;
}

_namespace_context* _namespace_context::make(
		const string &name, _namespace_context *context)
{

	_namespace_context *ns = new _namespace_context(name, context);
	return get_instance(ns);
}


string _namespace_context::to_string() const
{
	string res;
	if(context != NULL && !context->is_empty()) {
		res += context->to_string();
	}
	res += name;
	if(name != "") res += "::";

	return res;
}

void _namespace_context::compute_hash()
{
	hash_c = string_hash(name)*31;// + (size_t) context;
}

bool _namespace_context::operator==(const _namespace_context &rhs)
{

	if(this->name != rhs.name) return false;
	return this->context == rhs.context;


}

namespace_context namespace_context::get_toplevel_namespace()
{
	_namespace_context* cur = ns;
	_namespace_context* base = NULL;
	while(cur != NULL)
	{
		base = cur;
		cur = cur->context;
	}

	if(base == NULL) return namespace_context();
	return namespace_context(base);


}


void* namespace_context::get_ns() const
{
	return ns;
}

size_t _namespace_context::nc_hash::operator()(_namespace_context *ns) const
{
	return ns->get_hash_code();
}

bool _namespace_context::nc_eq::operator()(_namespace_context *a,
		_namespace_context *b) const
{
	return *a == *b;
}


}
