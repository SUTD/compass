/*
 * MemoryAnalysis.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef MEMORYANALYSIS_H_
#define MEMORYANALYSIS_H_
#include "sail/Function.h"
#include "Constraint.h"
#include "call_id.h"



#include <map>
#include <set>
#include <vector>
using namespace std;


namespace sail {
class Block;
class BasicBlock;
class SuperBlock;
class Assignment;
class FieldRefRead;
class FieldRefWrite;
class Load;
class Store;
class ArrayRefRead;
class ArrayRefWrite;
class AddressVar;
class Binop;
class Unop;
class Cast;
class DropVariable;
class StaticAssert;
class CfgEdge;
class AddressString;
class FunctionCall;
class FunctionPointerCall;
class Assume;
class AssumeSize;
class AddressLabel;
class InstanceOf;
}

class SummaryGraph;
class MemoryLocation;
class MemoryGraph;
class Error;
class BlockExecutionCounter;
class SummaryFetcher;
class AnalysisResult;

#include "Analysis.h"
#include <boost/serialization/access.hpp>
using namespace std;

struct dotty_info
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & level;
		ar & key;
		ar & val1;
		ar & val2;
	}
	dotty_info(int level, const string & key, const string & val1,
			const string & val2);
	dotty_info(){}
	int level;
	string key;
	string val1;
	string val2;
};

/*
 * Represents the constraint associated with each basic block at the beginning
 * of the block and at the end. The constraint can change within a basic block
 * due to exit function calls.
 */
struct block_constraint
{
	Constraint begin_c;
	Constraint end_c;
};

class MemoryAnalysis: public Analysis {
	friend class Instantiator;
	friend class ClientAnalysisManager;
private:
	MemoryGraph* mg;
	vector<string> processed_instructions;
	set<sail::Block*> processed_blocks;
	Constraint stmt_guard;
	sail::Block* cur_block; // the currently analyzed block
	map<sail::Block*, block_constraint> block_to_stmt_guard;

	vector<dotty_info>* dotties;
	int dotty_level;

	map<sail::CfgEdge*, Constraint> edge_constraints;
	map<int, int> alloc_per_lines;
	sail::SummaryUnit* su;

	/*
	 * The set of locations deleted by the analysis
	 * Any access paths in
	 */
	set<MemoryLocation*> deleted_locs;

	/*
	 * The condition under which we enter the loop body.
	 */
	Constraint entry_cond;

	int instruction_number;

	sail::Block* loop_entry_block;

	Constraint return_cond;

	BlockExecutionCounter* bec;

	SummaryFetcher* sf;

	int cg_id;

	map<il::type*, set<call_id> >* function_addresses;

	bool track_rtti;




public:

	virtual analysis_order get_order();
	virtual analysis_kind get_analysis_kind();
	virtual sum_data_type get_summary_type();
	virtual void register_summary_callbacks();

	bool has_summary();
	MemoryAnalysis();
	SummaryGraph* do_analysis(sail::SummaryUnit *su,bool report_errors,
			SummaryFetcher * sf, AnalysisResult *ar,
			vector<dotty_info>* dotties, int cg_id,
			map<il::type*, set<call_id> >* function_addresses,
			bool track_rtti);



	virtual SummaryGraph* do_analysis(sail::SummaryUnit *su,bool report_errors,
			AnalysisResult *ar, SummaryFetcher * sf, int cg_id,
			map<il::type*, set<call_id> >* function_addresses,
			bool track_rtti);
	virtual ~MemoryAnalysis();
	vector<string> & get_processed_instructions();

	/*
	 * Substitutes old_ap with new_ap in statement guards.
	 * This might be necessary when certain access paths are upgraded to
	 * array pointers rather than just pointers.
	 */
	void update_statement_guards(Term* old_ap, Term* new_ap);

	Constraint get_current_stmt_guard();

	inline bool track_dynamic_type() {return track_rtti;};



	int get_cur_block_id();
	int get_cur_iteration_counter();
	sail::SummaryUnit* get_su();
	BlockExecutionCounter* get_bec();
	/*
	 * end specifies whether you want the constraint at the end
	 * or in the beginning.
	 */
	Constraint get_stmt_guard(sail::Block* b, bool end);

	int get_cg_id();

	static string escape_dotty_string(const string & s);

	void add_dotty(const string & label, const string & dotty);

	/*
	 * Add a dotty heading
	 */
	void push_dotty_level(const string & section, const string & info = "");
	void pop_dotty_level();

   MemoryGraph* get_mg();


private:
	void process_blocks(vector<sail::Block*> & blocks);

	void fill_blocks(sail::Block* cur, vector<sail::Block*> & to_fill,
			set<sail::Block*> & visited);

	void process_basic_block(sail::BasicBlock* b);
	void process_super_block(sail::SuperBlock* sb);
	void process_statement(sail::Instruction *inst);

	void process_assignment(sail::Assignment *inst);
	void process_field_ref_read(sail::FieldRefRead* inst);
	void process_array_ref_read(sail::ArrayRefRead* inst);
	void process_field_ref_write(sail::FieldRefWrite* inst);
	void process_array_ref_write(sail::ArrayRefWrite* inst);
	void process_load(sail::Load* inst);
	void process_store(sail::Store* inst);
	void process_address_var(sail::AddressVar* inst);
	void process_unop(sail::Unop* inst);
	void process_binop(sail::Binop* inst);
	void process_cast(sail::Cast* c);
	void process_address_string(sail::AddressString* as);
	void process_memory_allocation(sail::FunctionCall* fc);
	void process_memory_deallocation(sail::FunctionCall* fc);
	void process_static_assert(sail::StaticAssert* inst);
	void process_function_call(sail::FunctionCall* inst);
	void process_function_pointer_call(sail::FunctionPointerCall* inst);
	void process_virtual_method_call(sail::FunctionCall* inst);
	void process_assume(sail::Assume* assume);
	void process_assume_size(sail::AssumeSize* inst);
	void process_address_label(sail::AddressLabel* adr_label);
	void process_instance_of(sail::InstanceOf* inst);
	void process_init_functions(vector<SummaryGraph*>& init_summaries);


	void compute_edge_constraints(sail::Block* b);
	void update_statement_guard(Constraint c);

	void compute_loop_entry_block(sail::SuperBlock* su);

	/*
	 * We add stack layout assumptions to the background; this
	 * is necessary to rule out various entry aliasing
	 * relations. E.g., If x is a stack-allocated array with 32 bytes,
	 * then a constraint like &x + 4 = &y is not satisfiable.
	 */
	void add_stack_layout_assumptions();




	void add_dotty(sail::Instruction *inst);
	void add_dotty(const string & label, set<AccessPath*>& aps);
	void add_dotty(const string & label, AccessPath* ap1);
	void add_dotty(const string & label, AccessPath* ap1, AccessPath* ap2);
	void add_dotty(const string & label, AccessPath* ap1, AccessPath* ap2,
			AccessPath* ap3);

	string dotty_from_info(const string& info);




};
class CompareBlockId:public binary_function<sail::CfgEdge*, sail::CfgEdge*, bool> {
public:
	bool operator()(const sail::CfgEdge* b1, const sail::CfgEdge* b2) const;
};

#endif /* MEMORYANALYSIS_H_ */
