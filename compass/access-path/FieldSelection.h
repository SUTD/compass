/*
 * FieldSelection.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef FIELDSELECTION_H_
#define FIELDSELECTION_H_

#include "AccessPath.h"
#include "IndexVariable.h"
#include "FunctionTerm.h"


#define SIZE_OFFSET -4
#define SIZE_FIELD_NAME "size"

#define RTTI_OFFSET -8
#define RTTI_FIELD_NAME "rtti"

#define DELETED_OFFSET (-12)
#define DELETED_FIELD_NAME "is_deleted"

class FieldSelection: public AccessPath, public FunctionTerm {
	friend class AccessPath;
	friend class boost::serialization::access;
private:
	AccessPath* inner;
	AccessPath* index_var; //if recursive
	string field_name;
	int field_offset;
	bool anonymous;


	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & inner;
		ar & index_var;
		ar & field_name;
		ar & field_offset;
		ar & anonymous;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & inner;
		ar & index_var;
		ar & field_name;
		ar & field_offset;
		ar & anonymous;

		inner = AccessPath::to_ap(Term::get_term_nodelete(inner->to_term()));
		if(index_var != NULL)
			index_var = AccessPath::to_ap(Term::get_term_nodelete(
				index_var->to_term()));
		ar & boost::serialization::base_object<FunctionTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

protected:
	FieldSelection(const string & field_name, int field_offset,
			AccessPath* inner, il::type* t);
	FieldSelection(int field_offset, AccessPath* inner, il::type* t);
	FieldSelection(const string & field_name, int field_offset,
			AccessPath* inner, AccessPath* index_var,
			il::type* t);
	FieldSelection(){};
	virtual ~FieldSelection();
public:
	static FieldSelection* make(const string& field_name,
			int field_offset, il::type* t, AccessPath* inner,
			AccessPath* index_var = NULL);
	static FieldSelection* make(const string& field_name,
			int field_offset, AccessPath* inner,
			AccessPath* index_var = NULL);

	static FieldSelection* make_size(AccessPath* inner);
	static FieldSelection* make_rtti(AccessPath* inner);
	static FieldSelection* make_deleted(AccessPath* inner);

	bool is_size_field();
	bool is_rtti_field();
	bool is_deleted_field();

	/*
	 * Makes up an anynomous field, i.e., one who name is unknown.
	 */
	static FieldSelection* make_anonymous(int field_offset, il::type* t,
			AccessPath* inner);


	virtual string to_string();
	virtual AccessPath* get_inner();

	/* returns NULL if not recursive.
	 */
	virtual IndexVariable* get_index_var();
	virtual AccessPath* get_index_expression();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);


	int get_field_offset();
	string get_field_name();
	bool is_anonymous();
private:
	static il::type* make_type(AccessPath* inner, int field_offset);

};

#endif /* FIELDSELECTION_H_ */
