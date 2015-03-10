/*
 * StringLiteral.h
 *
 *  Created on: Feb 3, 2010
 *      Author: isil
 */

#ifndef COMPASS_STRINGLITERAL_H_
#define COMPASS_STRINGLITERAL_H_

#include "AccessPath.h"
#include "FunctionTerm.h"
#include "ConstantValue.h"

#include <map>
using namespace std;




class StringLiteral: public AccessPath, public FunctionTerm
{
	friend class AccessPath;
	friend class FunctionTerm;
	friend class boost::serialization::access;

private:
	string s;
	static map<string, int> string_to_id;
	static int counter;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & s;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & s;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
		int new_id = get_id(s);
		AccessPath* new_arg = ConstantValue::make(new_id);
		this->args[0] = new_arg->to_term();
		compute_hash_code();


	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()



protected:
	StringLiteral(const string& string_literal);
	StringLiteral(){};
	virtual ~StringLiteral();
	static int get_id(const string& s);
	static void clear();
public:
	static StringLiteral* make(const string& string_literal);
	const string& get_string_constant();
	virtual string to_string();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);

};

#endif /* COMPASS_STRINGLITERAL_H_ */
