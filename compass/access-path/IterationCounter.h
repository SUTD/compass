/*
 * IterationCounter.h
 *
 *  Created on: Oct 14, 2009
 *      Author: tdillig
 */

#ifndef ITERATIONCOUNTER_H_
#define ITERATIONCOUNTER_H_

#include "AccessPath.h"
#include "VariableTerm.h"

class IterationCounter: public AccessPath, public VariableTerm {
	friend class AccessPath;
private:
	int id;
	bool parametric;
	string context;
	int num_contexts;
	int num_total_contexts; //always includes loops
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & id;
		ar & parametric;
		ar & context;
		ar & num_contexts;
		ar & num_total_contexts;
		ar & boost::serialization::base_object<VariableTerm>(*this);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<AccessPath>(*this);
		ar & id;
		ar & parametric;
		ar & context;
		ar & num_contexts;
		ar & num_total_contexts;
		ar & boost::serialization::base_object<VariableTerm>(*this);

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
public:

	static IterationCounter* make_parametric(int id);
	static IterationCounter* make_last(int id);
	static IterationCounter* make_parametric(IterationCounter* v);
	static IterationCounter* make_last(IterationCounter* v);

	static IterationCounter* add_context(IterationCounter* ic,
			int inst_id, const string & context, bool inc_num_contexts);


	/*
	 * Makes an iteration counter that is identical to c, but its id is 0.
	 */
	static IterationCounter* normalize(IterationCounter* ic);

	virtual string to_string();
	int get_id();
	string get_context();
	int get_num_contexts();
	int get_num_total_contexts();
	virtual Term* substitute(map<Term*, Term*>& subs);
	virtual Term* substitute(Term* (*sub_func)(Term* t, void* data),
					void* my_data);
	bool is_parametric();
	bool is_last();
protected:
	IterationCounter(int id, bool is_parametric, const string & context,
			int num_contexts, int num_total_contexts);
	IterationCounter(){};
	virtual ~IterationCounter();
};

#endif /* ITERATIONCOUNTER_H_ */
