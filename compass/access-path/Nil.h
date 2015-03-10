/*
 * Nil.h
 *
 *  Created on: Feb 9, 2010
 *      Author: tdillig
 */

#ifndef NIL_H_
#define NIL_H_

#include "AccessPath.h"
#include "VariableTerm.h"

/*
 * The Nil access path is used to encode that no value is associated with
 * a key in an ADT.
 */

class Nil: public AccessPath, public VariableTerm
{
	friend class AccessPath;
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & boost::serialization::base_object<VariableTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & boost::serialization::base_object<VariableTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
public:
	static Nil* make();
	virtual string to_string();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
					void* my_data);

protected:
	Nil();
	virtual ~Nil();
};

#endif /* NIL_H_ */
