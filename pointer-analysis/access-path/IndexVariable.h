/*
 * IndexVariable.h
 *
 *  Created on: Sep 6, 2009
 *      Author: isil
 */

#ifndef INDEXVARIABLE_H_
#define INDEXVARIABLE_H_

#include"AccessPath.h"
#include "VariableTerm.h"

class IndexVariable: public AccessPath, public VariableTerm {

static int index_counter;
friend class boost::serialization::access;
template<class Archive>
void save(Archive & ar, const unsigned int version) const
{
	ar & boost::serialization::base_object<AccessPath>(*this);
	ar & index;
	ar & boost::serialization::base_object<VariableTerm>(*this);
}
template<class Archive>
void load(Archive & ar, const unsigned int version)
{
	ar & boost::serialization::base_object<AccessPath>(*this);
	ar & index;
	ar & boost::serialization::base_object<VariableTerm>(*this);

}
BOOST_SERIALIZATION_SPLIT_MEMBER()



private:
	int index;

public:
	static IndexVariable* make_source();
	static IndexVariable* make_target();
	static IndexVariable* make_free();
	static IndexVariable* make_source(IndexVariable* other);
	static IndexVariable* make_target(IndexVariable* other);
	static IndexVariable* make_free(IndexVariable* other);

	/*
	 * Instantiation-related index variables. These are only
	 * used while instantiating access paths.
	 */
	static IndexVariable* make_inst_source(IndexVariable* other);
	static IndexVariable* make_inst_target(IndexVariable* other);

	bool is_source();
	bool is_target();
	bool is_free();
	bool is_inst_source();
	bool is_inst_target();

	int get_index();
	virtual string to_string();

	static void clear();

	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);


protected:
	IndexVariable(ap_attribute attrib);
	IndexVariable(int index, ap_attribute attrib);
	IndexVariable(){};
	virtual ~IndexVariable();
};

#endif /* INDEXVARIABLE_H_ */
