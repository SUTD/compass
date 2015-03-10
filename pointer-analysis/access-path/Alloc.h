/*
 * Alloc.h
 *
 *  Created on: Oct 15, 2008
 *      Author: tdillig
 */

#ifndef ALLOC_H_
#define ALLOC_H_

#include "AccessPath.h"
#include "FunctionTerm.h"
#include "Identifier.h"
#include "ConstantValue.h"


struct info_item
{
	friend class boost::serialization::access;

	int line;
	int instruction_number;
	string function_name;
	string file;

	info_item(int line, int instruction_number, const Identifier& unit_id);
	info_item();
	string to_string() const;
	bool operator==(const info_item& other) const;
	bool operator!=(const info_item& other) const;
	bool operator<(const info_item& other) const;
	bool operator>(const info_item& other) const;



	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & line;
		ar & instruction_number;
		ar & function_name;
		ar & file;
	}
};

// --------------------------------------------------------

struct alloc_info;

struct alloc_info
{
	friend class boost::serialization::access;

	static map<alloc_info, int> info_ids;
	static int counter;

	vector<info_item> callstack;

	alloc_info(int line, int instruction_number, const Identifier& unit_id);
	alloc_info(const alloc_info& other);
	alloc_info();
	void push_caller(int line, int instruction_number, const Identifier& unit_id);
	const info_item& get_original_allocation_site() const;
	bool operator==(const alloc_info& other) const;
	bool operator!=(const alloc_info& other) const;
	bool operator<(const alloc_info& other) const;
	int get_id() const;


	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & callstack;
	}

	static void clear();

};

// --------------------------------------------------------------------

class Alloc:public AccessPath, public FunctionTerm {
	friend class AccessPath;
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & alloc_id;
		ar & index_var;
		ar & boost::serialization::base_object<FunctionTerm>(*this);
		assert(this->get_id() != 0);
		Alloc* a = (Alloc*)this;

	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & alloc_id;
		ar & index_var;
		if(index_var != NULL)
			index_var =
				AccessPath::to_ap(Term::get_term_nodelete(index_var->to_term()));
		ar & boost::serialization::base_object<FunctionTerm>(*this);
		cout << "inside load +=+++++++++ " <<endl;
		int new_id = alloc_id.get_id();
		AccessPath* new_arg = ConstantValue::make(new_id);
		this->args[0] = new_arg->to_term();
		compute_hash_code();


	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
	alloc_info alloc_id;
	AccessPath* index_var;
protected:
	Alloc(const alloc_info& alloc_id, il::type* t, AccessPath* index_var);
	Alloc(const alloc_info& alloc_id, il::type* t);


	virtual ~Alloc();
public:
	static Alloc* make(const alloc_info& alloc_id, il::type* t, bool is_nonnull);
	static Alloc* make(const alloc_info& alloc_id, il::type* t, AccessPath* index,
			bool is_nonnull);
	const alloc_info& get_alloc_id();

	virtual string to_string();
	virtual IndexVariable* get_index_var();
	virtual AccessPath* get_index_expression();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
			void* my_data);
	void add_attribute();

	bool is_nonnull();

};


#endif /* ALLOC_H_ */
