/*
 * ArithmeticValue.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef ARITHMETICVALUE_H_
#define ARITHMETICVALUE_H_

#include "AccessPath.h"
#include "Variable.h"
#include <vector>
#include "ArithmeticTerm.h"
using namespace std;

class ConstantValue;


struct offset
{
	friend class boost::serialization::access;
	// Field-related
	int field_offset;
	il::type* field_type;
	string field_name;

	// index-related
	long int elem_size;
	AccessPath* index;


	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & field_offset;
		ar & field_type;
		ar & field_name;
		ar & elem_size;
		ar & index;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & field_offset;
		ar & field_type;
		ar & field_name;
		ar & elem_size;
		ar & index;
		if(index != NULL)
			index = AccessPath::to_ap(Term::get_term_nodelete(index->to_term()));

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

	offset(long int elem_size, AccessPath* index)
	{
		this->elem_size= elem_size;
		this->index = index;
		this->field_offset = -1;
		this->field_type = NULL;
		this->field_name = "";
	}
	offset(const string& field_name, int field_offset, il::type* t)
	{
		this->field_offset = field_offset;
		this->field_type = t;
		this->field_name = field_name;
		elem_size = -1;
		index = NULL;
	}
	offset() {};

	bool operator==(const offset & o)
	{
		return field_offset == o.field_offset   && o.elem_size == elem_size
				&& index == o.index;
	}


	bool is_index_offset() const {return field_offset == -1;}
	bool is_field_offset() const {return field_offset != -1;};

};


class ArithmeticValue: public AccessPath, public ArithmeticTerm {
	friend class AccessPath;
	friend class boost::serialization::access;

private:

	/*
	 * Only non-null if this corresponds to structured pointer arithmetic.
	 * i.e., this arithmetic value obtained by taking the address of another
	 * access path? e.g. &a.f or &a[1]
	 */
	AccessPath* base_ptr;

	/*
	 * This is only non-empty for arithmetic values corresponding to
	 * structured pointer arithmetic.
	 */
	vector<offset> offsets;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & base_ptr;
		ar & offsets;
		ar & boost::serialization::base_object<ArithmeticTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & base_ptr;
		if(base_ptr != NULL)
			base_ptr =
				AccessPath::to_ap(Term::get_term_nodelete(base_ptr->to_term()));
		ar & offsets;
		ar & boost::serialization::base_object<ArithmeticTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
	ArithmeticValue(const map<Term*, long int>& elems, long int constant,
			il::type* t);

protected:
	ArithmeticValue(){};
	ArithmeticValue(AccessPath* ap1, long int c1, AccessPath* ap2, long int c2,
			il::type* t);
	ArithmeticValue(AccessPath* ap, long int c, il::type* t);
	ArithmeticValue(AccessPath* base_ptr, vector<offset>& offsets,
			const map<Term*, long int>& elems, long int constant, il::type* t);

	virtual ~ArithmeticValue();
public:
	static AccessPath* make_plus(AccessPath* ap1, AccessPath* ap2);
	static AccessPath* make_minus(AccessPath* ap1, AccessPath* ap2);
	static AccessPath* make_times(AccessPath* ap1, AccessPath* ap2);
	static AccessPath* make_division(AccessPath* ap1, AccessPath* ap2);
	static AccessPath* make(const map<Term*, long int>& elems, long int constant);
	static AccessPath* make_field_offset_plus(AccessPath* ap,
			const string& field_name, int offset, il::type*
			field_type);
	static AccessPath* make_index_offset_plus(AccessPath* ap, long int elem_size,
			AccessPath* index, il::type* t);
	static AccessPath* make_structured_pointer_arithmetic(AccessPath* base_ptr,
			vector<offset>& offsets);

	virtual bool is_structured_ptr_arithmetic();
	const vector<offset>& get_offsets();
	AccessPath* get_base_ptr();


	/*
	 * Calling this function only makes sense if this arithmetic value corresponds
	 * to pointer arithmetic such as p+4. It will return NULL otherwise.
	 */
	virtual AccessPath* get_inner();


	/*
	 * Is the given term one of terms of this arithmetic value?
	 */
	bool contains_term(AccessPath* term);

	/*
	 * Gives the coefficient of the given term in this arithmetic value,
	 * 0 if the term doesn't occur in this arithmetic value.
	 */
	long int get_coefficient(AccessPath* term);

	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
			void* my_data);

	virtual bool operator==(const Term& other);
	virtual string to_string();

private:
	/*
	 * Checks whether this is an actual arithmetic value or
	 * some other access path, such as a constant.
	 */
	static AccessPath* canonicalize(ArithmeticValue* av);
	static map<Term*, long int> elems_to_terms(map<AccessPath* ,long int>& elems);


};

#endif /* ARITHMETICVALUE_H_ */
