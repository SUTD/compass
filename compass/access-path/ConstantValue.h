/*
 * ConstantValue.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef CONSTANTVALUE_H_
#define CONSTANTVALUE_H_

#include "AccessPath.h"
#include "ConstantTerm.h"
#include <string>
using namespace std;

class ConstantValue:public AccessPath, public ConstantTerm {
	friend class boost::serialization::access;
	friend class AccessPath;
	friend class ConstantTerm;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & boost::serialization::base_object<ConstantTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & boost::serialization::base_object<ConstantTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
	ConstantValue(){}
public:

	ConstantValue(long int value);
	virtual ~ConstantValue();

public:
	static ConstantValue* make(long int value);
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);
};

#endif /* CONSTANTVALUE_H_ */
