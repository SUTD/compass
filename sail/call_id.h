/*
 * call_id.h
 *
 *  Created on: Apr 12, 2010
 *      Author: tdillig
 */

#ifndef CALL_ID_H_
#define CALL_ID_H_

#include <string>
using namespace std;


#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "namespace_context.h"
#include "language.h"

class call_id;
namespace il {
	class type;

}

class Identifier;

struct call_id
{
	friend class boost::serialization::access;

	call_id(const Identifier& id);
	call_id(const call_id & other);
	call_id(const string & name, const il::namespace_context& ns, il::type* sig,
			language_type language);

	call_id();

	// constructor for loops
	call_id(int loop_id, const string& fn_name, const il::namespace_context& ns,
			il::type* sig, language_type language);
	string name;
	il::type* sig;
	int loop_id;
	il::namespace_context ns;
	language_type language;


	const string& get_function_name();
	il::type* get_signature() const;
	const il::namespace_context& get_namespace();

	bool operator==(const call_id & other) const;
	bool operator<(const call_id& other) const;
	string to_string() const;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & name;
		ar & ns;
		ar & sig;
		ar & loop_id;
		ar & language;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & name;
		ar & ns;
		ar & sig;
		ar & loop_id;
		ar & language;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

};

#endif /* CALL_ID_H_ */
