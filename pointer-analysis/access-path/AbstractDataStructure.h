/*
 * AbstractDataStructure.h
 *
 *  Created on: Feb 1, 2010
 *      Author: isil
 */

#ifndef ABSTRACTDATASTRUCTURE_H_
#define ABSTRACTDATASTRUCTURE_H_

#include "AccessPath.h"
#include "FunctionTerm.h"
#include "type.h"

enum abstract_data_type
{
	POSITION_DEPENDENT,
	SINGLE_VALUED,
	MULTI_VALUED
};

class AbstractDataStructure: public AccessPath, public FunctionTerm
{
	friend class AccessPath;
	friend class boost::serialization::access;
private:
	AccessPath* inner;
	AccessPath* index_var;
	abstract_data_type adt_type;
	il::type* key_type;
	il::type* value_type;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & inner;
		ar & index_var;
		ar & adt_type;
		ar & key_type;
		ar & value_type;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & inner;
		inner = AccessPath::to_ap(Term::get_term_nodelete(inner->to_term()));
		ar & index_var;
		index_var =
				AccessPath::to_ap(Term::get_term_nodelete(index_var->to_term()));
		ar & adt_type;
		ar & key_type;
		il::type::register_loaded_typeref(&key_type);
		ar & value_type;
		il::type::register_loaded_typeref(&value_type);
		ar & boost::serialization::base_object<FunctionTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

protected:
	AbstractDataStructure(AccessPath* inner, AccessPath* index_var,
			il::type* key_type, il::type* value_type, il::type* t);
	AbstractDataStructure(){};
	virtual ~AbstractDataStructure();

public:
	static AbstractDataStructure* make(AccessPath* inner, AccessPath* index_var,
			 il::type* key_type, il::type* value_type, il::type* t);
	virtual string to_string();
	virtual AccessPath* get_inner();
	virtual IndexVariable* get_index_var();
	virtual AccessPath* get_index_expression();
	il::type* get_key_type();
	il::type* get_value_type();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);
	bool is_position_dependent();
	bool is_value_dependent();
	bool is_single_valued();
	bool is_multi_valued();
	AccessPath* get_index_from_key(AccessPath* key);

	virtual bool operator==(const Term& other);

};

#endif /* ABSTRACTDATASTRUCTURE_H_ */
