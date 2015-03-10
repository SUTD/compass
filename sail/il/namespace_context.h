

#ifndef NAMESPACE_CONTEXT_H_
#define NAMESPACE_CONTEXT_H_

#include <unordered_set>

#include "hash.h"
#include <set>

using namespace std;

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

namespace il {


/*****************This is internally used by namespace_context *********/

class _namespace_context;

/**
 * Represents a namespace by an unspecified name and a pointer to
 * the enclosing namespace.  Pointers to _namespace_context are unique.
 */
class _namespace_context {
	friend class namespace_context;
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & name;
        ar & context;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & name;
        ar & context;
        context = _namespace_context::get_instance(context);

        if(name == "") assert(context == NULL);
        if(context != NULL) assert(context->name != "");

        compute_hash();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    bool is_empty();
public:
	static void delete_unused_instances();

private:
    string name;
    _namespace_context *context;
    size_t hash_c;
    struct nc_hash {
		size_t operator()(_namespace_context *ns) const;
	};

	struct nc_eq {
		bool operator()(_namespace_context *a, _namespace_context *b) const;
	};
	static unordered_set<_namespace_context *, nc_hash, nc_eq> instances;

    static _namespace_context* get_instance(_namespace_context* ns);
	static _namespace_context *make(const string &name, _namespace_context *context);

	string to_string() const;
	bool operator==(const _namespace_context &rhs);
    inline size_t get_hash_code() {return hash_c;}
    _namespace_context(const string &aname, _namespace_context *acontext);
    _namespace_context();
    ~_namespace_context();
    void compute_hash();



	static unordered_set<_namespace_context*> unused_instances;


};

/***************The actual interface*********/

class namespace_context
{
	static bool _init;
	static bool init();
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & ns;
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & ns;
		ns = _namespace_context::get_instance(ns);
	}



	BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
	_namespace_context* ns;





public:
	namespace_context(_namespace_context* ns);
	namespace_context();
	namespace_context(const namespace_context & ns);
	namespace_context(const string &name, const namespace_context &context);
	namespace_context(const string &name);
	string to_string() const;
	bool operator==(const namespace_context &rhs) const;
	bool operator!=(const namespace_context& other) const;
	bool operator<(const namespace_context& other) const;
	bool operator>(const namespace_context& other) const;
	const string& get_name();
	namespace_context get_context();
	size_t get_hash_code();
	void* get_ns() const;

	/**
	 * @return The highest level namespace.
	 * E.g., for X::Y::Z, this would be X.
	 */
	namespace_context get_toplevel_namespace();


};



}

#endif /* NAMESPACE_CONTEXT_H_ */
