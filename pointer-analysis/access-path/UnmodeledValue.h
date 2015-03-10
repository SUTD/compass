/*
 * UnmodeledValue.h
 *
 *  Created on: Sep 6, 2009
 *      Author: tdillig
 */

#include "FunctionTerm.h"
#include "AccessPath.h"
#include "DisplayTag.h"

#ifndef UNMODELEDVALUE_H_
#define UNMODELEDVALUE_H_

enum unmodeled_type
{
	UNM_IMPRECISE,
	UNM_UNINIT,
	UNM_INVALID,
	UNM_UNKNOWN_FUNCTION


};

class UnmodeledValue: public AccessPath, public FunctionTerm {
private:
	static int unmodeled_ap_counter;
	AccessPath* index_var;
	DisplayTag dt;

	unmodeled_type kind;
	int id;

	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & kind;
		ar & index_var;
		ar & id;
		ar & dt;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & kind;
		ar & index_var;
		ar & id;
		ar & dt;
		ar & boost::serialization::base_object<FunctionTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
protected:
	UnmodeledValue(il::type* t, unmodeled_type kind, const DisplayTag & dt);
	UnmodeledValue(il::type* t, AccessPath* iv, unmodeled_type kind,
			const DisplayTag& dt);
	UnmodeledValue(int id, il::type* t, AccessPath* iv, unmodeled_type kind,
			const DisplayTag& dt);
	UnmodeledValue(){};
	virtual ~UnmodeledValue();

public:
	virtual string to_string();
	static UnmodeledValue* make(il::type* t, unmodeled_type kind,
			const DisplayTag & dt);
	static UnmodeledValue* make(il::type* t, AccessPath* iv,
			unmodeled_type kind, const DisplayTag & dt);
	static UnmodeledValue* make(int id, il::type* t, AccessPath* iv,
			unmodeled_type kind, const DisplayTag & dt);
	static UnmodeledValue* make_imprecise(il::type* t, const DisplayTag & dt);
	static UnmodeledValue* make_imprecise(il::type* t, AccessPath* iv,
			const DisplayTag & dt);
	static UnmodeledValue* make_invalid(il::type* t,
			const DisplayTag& dt);
	static UnmodeledValue* make_invalid(il::type* t, AccessPath* iv,
			const DisplayTag & dt);
	static UnmodeledValue* make_uninit(il::type* t,
			const DisplayTag & dt);
	static UnmodeledValue* make_uninit(il::type* t, AccessPath* iv,
			const DisplayTag & dt);
	static UnmodeledValue* make_unknown_fn_return(il::type* t,
			const DisplayTag & dt);
	static UnmodeledValue* make_unknown_fn_return(il::type* t, AccessPath* iv,
			const DisplayTag & dt);
	static void clear();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
					void* my_data);
	virtual bool operator==(const Term& other);
	bool is_imprecise();
	bool is_uninitialized();
	bool is_constant_deref();
	bool is_unknown_func_return();
	unmodeled_type get_kind();
	int get_unmodeled_id();

	virtual IndexVariable* get_index_var();
	virtual AccessPath* get_index_expression();
	const DisplayTag& get_display_tag();
};

#endif /* UNMODELEDVALUE_H_ */
