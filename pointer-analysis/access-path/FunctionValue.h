/*
 * FunctionValue.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef FUNCTIONVALUE_H_
#define FUNCTIONVALUE_H_

#include "AccessPath.h"
#include <vector>
#include "FunctionTerm.h"

using namespace std;

enum ap_fun_type{
	FN_BITWISE_NOT,
	FN_BITWISE_AND,
	FN_BITWISE_OR,
	FN_BITWISE_XOR,
	FN_LEFT_SHIFT,
	FN_RIGHT_SHIFT,
	FN_MOD, //6
	FN_DISJOINT,
	FN_TARGET,
	FN_UNMODELED,
	FN_LENGTH,
	FN_ADT_POS_SELECTOR,
	FN_IS_NIL,
	FN_TIMES, //13
	FN_DIVIDE, //14
	FN_SUBTYPE,
	FN_SHARED_BASELOC,
	FN_UNINTERPRETED
};

class ArrayRef;
class AbstractDataStructure;

class FunctionValue: public AccessPath, public FunctionTerm {
	friend class AccessPath;
	friend class ArithmeticValue;
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & arguments;
		ar & fn_type;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & arguments;
		ar & fn_type;
		for(unsigned int i=0; i < arguments.size(); i++)
		{
			arguments[i] = AccessPath::to_ap(
					Term::get_term_nodelete(arguments[i]->to_term()));
		}
		ar & boost::serialization::base_object<FunctionTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
	vector<AccessPath*> arguments;
	ap_fun_type fn_type;
	static int imprecise_id;
	static map< pair<AccessPath*, AccessPath*>, int> shared_bases;
public:
	FunctionValue(ap_fun_type apt, const string& id,
			const vector<AccessPath*>& args, il::type* t,
			bool invertible, int attribute);

protected:
	FunctionValue(ap_fun_type apt, const string & id, AccessPath* arg1,
			AccessPath* arg2, il::type* t, bool invertible, int attribute);
	FunctionValue(ap_fun_type apt, const string & id, AccessPath* arg,
			il::type* t, bool invertible, int attribute);

	FunctionValue(ap_fun_type apt, int id,
			const vector<AccessPath*>& args, il::type* t,
			bool invertible, int attribute);
	FunctionValue(ap_fun_type apt, const string& id,
			const vector<AccessPath*>& args,
			const vector<Term*>& term_args, il::type* t,
			bool invertible, int attribute);
	FunctionValue(){};
	virtual ~FunctionValue();
	static AccessPath* _make_times(AccessPath* ap1, AccessPath* ap2);
	static AccessPath* _make_division(AccessPath* ap1, AccessPath* ap2);
public:
	static FunctionValue* make(ap_fun_type special_fun,
			AccessPath* ap);
	static AccessPath* make(ap_fun_type special_fun,
			AccessPath* ap1, AccessPath* ap2);
	static AccessPath* make(ap_fun_type special_fun,
			const vector<AccessPath*>& args, int attribute);
	static FunctionValue* make_disjoint(int id, AccessPath* ap, il::type* t);
	static FunctionValue* make_target(int id, AccessPath* ap);
	static FunctionValue* make_adt_position_selector(AccessPath* abs,
			AccessPath* key);




	static AccessPath* make(ap_fun_type apt, int id,
				const vector<AccessPath*>& args, il::type* t, int attribute);
	static AccessPath* make_length(AccessPath* arg);
	static AccessPath* make_is_nil(AccessPath* ap);
	static AccessPath* make_times(AccessPath* ap1, AccessPath* ap2);
	static AccessPath* make_division(AccessPath* ap1, AccessPath* ap2);
	static AccessPath* make_subtype(AccessPath* ap1, AccessPath* ap2);
	static AccessPath* make_shared_base(AccessPath* ap1, AccessPath* ap2);


	string get_function_name();
	vector<AccessPath*>& get_arguments();

	bool contains_term(AccessPath* term);
	virtual string to_string();
	ap_fun_type get_fun_type();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data);
	bool is_disjoint();
	bool is_target();
	bool is_imprecise();
	bool is_length();
	bool is_nil_function();
	bool is_multiplication();
	bool is_division();
	bool is_subtype_predicate();
	bool is_adt_pos_selector();
	bool is_shared_base_predicate();
	static void clear();

	//haiyan
	virtual Term* strip_term(Term*& t);

private:
	static string fun_type_to_string(ap_fun_type t);
	static string fun_type_to_prefix(ap_fun_type t);
	static vector<Term*> args_to_terms(const vector<AccessPath*>& args);
	static bool is_locally_allocated(AccessPath* ap);

};

#endif /* FUNCTIONACCESSPATH_H_ */
