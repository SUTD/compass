/*
 * Deref.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef DEREF_H_
#define DEREF_H_

#include "AccessPath.h"
#include "FunctionTerm.h"

class Deref: public AccessPath, public FunctionTerm {
	friend class AccessPath;
	friend class boost::serialization::access;
private:
	AccessPath* inner;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & inner;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & inner;
		inner = AccessPath::to_ap(Term::get_term_nodelete(inner->to_term()));
		ar & boost::serialization::base_object<FunctionTerm>(*this);


	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
protected:
	Deref(AccessPath* inner, il::type* t = NULL);
	Deref(){};
	virtual ~Deref();
public:
	static Deref* make(AccessPath* inner, il::type* t=NULL);
	virtual string to_string();

	/**
	 * Returns the access path that is dereferenced.
	 */
	virtual AccessPath* get_inner();

	virtual Term* substitute(map<Term*, Term*>& subs);

	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);
};

#endif /* DEREF_H_ */

