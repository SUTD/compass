/*
 * DeletedAccessTrace.h
 *
 *  Created on: May 13, 2010
 *      Author: isil
 */

#ifndef DELETEDACCESSTRACE_H_
#define DELETEDACCESSTRACE_H_


#include "ErrorTrace.h"

class DeletedAccessTrace;

struct deleted_access_lt
{
	bool operator()(const DeletedAccessTrace* ddt1,
			const DeletedAccessTrace* ddt2) const;
};

Term* replace_deleted_field(Term* t, void* data);

class DeletedAccessTrace: public ErrorTrace {
	friend class boost::serialization::access;
	friend class deleted_access_lt;

private:
	static set<DeletedAccessTrace*, deleted_access_lt> deleted_access_traces;

	/*
	 * representative access path of the deleted location
	 */
	AccessPath* accessed_loc;
	set<access_info> info;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & accessed_loc;
		ar & info;
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & accessed_loc;
		ar & info;
		accessed_loc = AccessPath::to_ap(
						Term::get_term_nodelete(accessed_loc->to_term()));
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
public:
	static DeletedAccessTrace* make(AccessPath* accessed_loc, string expression,
			int line, string file, Constraint fail_c, int counter_id);

	static DeletedAccessTrace* make(AccessPath* accessed_loc, Constraint fail_c,
			int counter_id, set<access_info>& info);

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
	DeletedAccessTrace();
	DeletedAccessTrace(AccessPath* deleted_memory, int counter_id);
	virtual ~DeletedAccessTrace();

private:
	static DeletedAccessTrace* get_shared(DeletedAccessTrace* ddt);

	static bool check_eligible(AccessPath* ap);
};



#endif /* DELETEDACCESSTRACE_H_ */
