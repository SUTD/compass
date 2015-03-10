/*
 * Address.h
 *
 *  Created on: Sep 27, 2008
 *      Author: tdillig
 */

#ifndef ADDRESS_H_ 
#define ADDRESS_H_

#include "AccessPath.h"
#include "FunctionTerm.h"
#include "ConstantValue.h"

class Address:public AccessPath, public FunctionTerm {
	friend class AccessPath;
	friend class boost::serialization::access;
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
		if(inner != NULL)
			inner = AccessPath::to_ap(Term::get_term_nodelete(inner->to_term()));
		ar & boost::serialization::base_object<FunctionTerm>(*this);
		int new_id = get_id(inner);
		AccessPath* new_arg = ConstantValue::make(new_id);
		this->args[0] = new_arg->to_term();
		compute_hash_code();

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
	AccessPath* inner;
	static map<AccessPath*, int> addr_to_id;
	static int counter;
protected:
	Address(AccessPath* inner);
	Address() {};
	virtual ~Address();
	static int get_id(AccessPath* ap);
	static void clear();
public:
	static Address* make(AccessPath* inner);
	virtual AccessPath* get_inner();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
			void* my_data);
	virtual string to_string();
	virtual bool operator==(const Term& other);

};

#endif /* ADDRESS_H_ */
