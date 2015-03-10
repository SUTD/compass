/*
 * CastErrorTrace.h
 *
 *  Created on: May 2, 2010
 *      Author: isil
 */

#ifndef CASTERRORTRACE_H_
#define CASTERRORTRACE_H_

#include "ErrorTrace.h"

#include <set>
using namespace std;

class CastErrorTrace;

struct cast_error_lt
{
  bool operator()(const CastErrorTrace* ce1, const CastErrorTrace* ce2) const;
};


Term* replace_rtti_field(Term* t, void* data);

class CastErrorTrace:public ErrorTrace {
	friend class boost::serialization::access;
	friend class cast_error_lt;

private:

	static set<CastErrorTrace*, cast_error_lt> cast_error_traces;

	AccessPath* memory_loc;
	il::type* static_type;
	set<access_info> info;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & memory_loc;
		ar & static_type;
		ar & info;
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & memory_loc;
		ar & static_type;
		il::type::register_loaded_typeref(&static_type);
		ar & info;

		memory_loc = AccessPath::to_ap(
						Term::get_term_nodelete(memory_loc->to_term()));
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
	static CastErrorTrace* make(AccessPath* accessed_memory,
			il::type* static_type, string expression, int line,
			string file, Constraint fail_c,  int counter_id);

	static CastErrorTrace* make(AccessPath* accessed_memory,
			il::type* static_type, Constraint fail_c, int counter_id,
			set<access_info>& info);

	void push_caller(callstack_entry& ce);
	virtual void report_error(vector<Error*>& errors, const Identifier & id);
	virtual string to_string();
	virtual string to_error();
	virtual string get_id();
	virtual int get_line() {return -1;};
	virtual ErrorTrace* replace(AccessPath* ap, AccessPath* replacement);

	virtual void instantiate_trace(Instantiator& inst,
			set<ErrorTrace*>& inst_traces, bool top_level_function);
	virtual void get_access_paths_to_instantiate(set<AccessPath*>& aps);

	virtual ErrorTrace* set_counter(int counter_id);


protected:
	CastErrorTrace(){};
	CastErrorTrace(AccessPath* accessed_memory,
			il::type* static_type, int counter_id);
	virtual ~CastErrorTrace();
private:
	static CastErrorTrace* get_shared(CastErrorTrace* ct);
};

#endif /* CASTERRORTRACE_H_ */
