/*
 * Variable.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef AP_VARIABLE_H_
#define AP_VARIABLE_H_
#include "AccessPath.h"
#include "VariableTerm.h"


namespace sail{
	class Variable;

};




class Variable: public AccessPath, public VariableTerm {
	friend class AccessPath;
	friend class Term;

	static int counter;
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & name;
		ar & sail_var;
		ar & su_id;
		ar & boost::serialization::base_object<VariableTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & name;
		ar & sail_var;
		ar & su_id;
		ar & boost::serialization::base_object<VariableTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
	string name;
	sail::Variable* sail_var;
	int su_id;

	static void set_su_id(int id)
	{
		cur_su_id = id;
	}


public:
	static int cur_su_id;
	static int e_counter;

	Variable(sail::Variable* var, il::type* t, ap_attribute attrib);
	Variable(string name, il::type* t, ap_attribute attrib);
	Variable(){};

	virtual ~Variable();

public:
	static Variable* make(const string & name, il::type* t);
	static Variable* make_program_var(sail::Variable* var);
	static Variable* make_program_var(sail::Variable* var, il::type* t);
	static Variable* make_disjointness_var(int id, il::type* t);
	static Variable* make_target_var(int id);
	static Variable* make_sum_temp(il::type* t);
	static Variable* make_string_const_var(const string& id);
	static Variable* make_temp(const string & name);
	static Variable* make_temp(il::type* t);
	static Variable* make_loop_error_temp(const string & sum_id);
	static Variable* make_client_var(const string& name, il::type* t);
	static void clear();

	virtual string to_string();
	string get_name();
	sail::Variable* get_original_var();

	void set_type(il::type* t){this->t = t;}

	bool is_local_var();
	bool is_global_var();
	bool is_argument();
	bool is_return_var();
	int get_arg_number();
	bool is_loop_exit_var();
	bool is_background_target_var();
	bool is_loop_error_temp();
	bool is_anonymous();






	/*
	 * Is this variable something made up by the analysis, such as
	 * disjointness vars, iteration counters etc.?
	 */
	bool is_synthetic();


	/*
	 * Is this variable introduced for the purpose of enforcing
	 * existence and uniqueness of edge constraints?
	 */
	bool is_disjointness_var();

	bool is_string_const();


	/*
	 * Is this a (scratch) temporary ? This should never stay in
	 * any constraints.
	 */
	bool is_temporary();

	/*
	 * Is this variable introduced to help with instantiating access paths?
	 */
	bool is_summary_temp();

	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
					void* my_data);

	virtual bool operator==(const Term& __other);





};

#endif /* AP_VARIABLE_H_ */
