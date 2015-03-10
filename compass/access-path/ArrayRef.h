/*
 * ArrayRef.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef ARRAYREF_H_
#define ARRAYREF_H_

#include "AccessPath.h"
#include "Variable.h"
#include "FunctionTerm.h"

class IndexVariable;



class ArrayRef: public AccessPath, public FunctionTerm {
	friend class AccessPath;
	friend class boost::serialization::access;
private:
	AccessPath* inner;
	AccessPath* index_var;
	bool is_string_const;
	int elem_size;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & inner;
		ar & index_var;
		ar & is_string_const;
		ar & elem_size;
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
		ar & is_string_const;
		ar & elem_size;
		ar & boost::serialization::base_object<FunctionTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

protected:
	ArrayRef(AccessPath* inner, AccessPath* index_var, bool is_string_const,
			il::type* t, int elem_size);
	ArrayRef(AccessPath* inner, AccessPath* index_var, bool is_string_const,
			int elem_size);
	ArrayRef(){};
	virtual ~ArrayRef();

public:
	static ArrayRef* make(AccessPath* inner, AccessPath* index_var,
			il::type* t, int elem_size);
	static ArrayRef* make_stringconst(AccessPath* inner,
			AccessPath* index_var, il::type* t);
	static ArrayRef* make(AccessPath* inner, AccessPath* index_var,
			int elem_size);
	int get_elem_size();
	virtual string to_string();
	virtual AccessPath* get_inner();
	virtual IndexVariable* get_index_var();
	virtual AccessPath* get_index_expression();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);
	bool is_string_constant();

};

#endif /* ARRAYREF_H_ */
