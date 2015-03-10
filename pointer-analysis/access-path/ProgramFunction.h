/*
 * ProgramFunction.h
 *
 *  Created on: Feb 19, 2010
 *      Author: isil
 */

#ifndef PROGRAMFUNCTION_H_
#define PROGRAMFUNCTION_H_

#include "AccessPath.h"
#include "FunctionTerm.h"
#include "ConstantValue.h"

/*
 * An access path corresponding to a function in the program.
 * This is used for modeling functions whose address is taken.
 */
class ProgramFunction:public AccessPath, public FunctionTerm
{
	friend class AccessPath;
	friend class FunctionTerm;
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & function_name;
		ar & signature;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & function_name;
		ar & signature;
		il::type::register_loaded_typeref(&signature);
		ar & boost::serialization::base_object<FunctionTerm>(*this);
		int new_id = get_id(function_name, signature);

		AccessPath* new_arg = ConstantValue::make(new_id);
		this->args[0] = new_arg->to_term();
		compute_hash_code();

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
	string function_name;
	il::type* signature;
	static map<string, int> function_name_to_id;
	static int counter;

protected:
	ProgramFunction(const string& function_name, il::type* t);
	ProgramFunction(){};
	virtual ~ProgramFunction();
public:
	static ProgramFunction* make(const string& function_name, il::type* t);
	static void clear();
	const string& get_function_name() const;
	il::type* get_signature();
	virtual string to_string();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);
private:
	int get_id(const string& s, il::type* t);
};

#endif /* PROGRAMFUNCTION_H_ */
