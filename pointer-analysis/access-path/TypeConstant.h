/*
 * TypeConstant.h
 *
 *  Created on: Apr 22, 2010
 *      Author: isil
 */

#ifndef TYPECONSTANT_H_
#define TYPECONSTANT_H_

#include "AccessPath.h"
#include "FunctionTerm.h"
#include "ConstantValue.h"

/*
 * An access path representing a type.
 * This is used for tracking dynamic type information.
 */
class TypeConstant: public AccessPath, public FunctionTerm
{
	friend class AccessPath;
	friend class FunctionTerm;
	friend class boost::serialization::access;


	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & type_constant;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & type_constant;
		il::type::register_loaded_typeref(&type_constant);
		ar & boost::serialization::base_object<FunctionTerm>(*this);
		int new_id = get_id(type_constant);

		AccessPath* new_arg = ConstantValue::make(new_id);
		this->args[0] = new_arg->to_term();
		compute_hash_code();

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
	il::type* type_constant;
	static map<string, int> type_to_id;
	static int counter;
	static set<TypeConstant*> added_axioms;



protected:
	TypeConstant(il::type* t);
	TypeConstant(){};
	virtual ~TypeConstant();
public:
	static TypeConstant* make(il::type* t);
	static void clear();
	il::type* get_type_constant() const;
	virtual string to_string();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);

	static void add_subtype_axioms(TypeConstant* tc);
	static void add_subtype_axioms_internal(TypeConstant* tc);
	static void add_all_axioms();

private:
	int get_id(il::type* t);


};

#endif /* TYPECONSTANT_H_ */
