/*
 * MemoryAnalysis.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "MemoryAnalysis.h"
#include "sail/Cfg.h"
#include "sail/BasicBlock.h"
#include "sail/Block.h"
#include "sail/CfgEdge.h"
#include "sail/sail.h"
#include "Error.h"
#include "Variable.h"
#include "SummaryGraph.h"
#include "MemoryGraph.h"
#include "BlockExecutionCounter.h"
#include "SummaryFetcher.h"
#include "AnalysisResult.h"
#include "SummaryStream.h"
#include "GlobalAnalysisState.h"
#include "Callgraph.h"
#include "compass_assert.h"
#include "access-path.h"
#include "ConstraintGenerator.h"
#include "ClientAnalysis.h"



#define DEBUG false


#define BUFFER_SIZE "buffer_size"
#define BUFFER_SAFE "buffer_safe"
#define ASSIGN "assign"
#define BUILTIN_MEMCPY "__builtin_memcpy"
#define OBSERVED "observed"

#define NIL_FN "get_nil"

#define IS_NIL_FN "is_nil"


MemoryAnalysis::MemoryAnalysis()
{
	dotty_level = 0;
	depedent_sum_types.insert(COMPASS_SUMMARY);
	cur_block = NULL;
	instruction_number = 0;
	loop_entry_block = NULL;
	bec = NULL;
	sf = NULL;
	ClientAnalysis::cam.register_memory_analysis(this);
}

analysis_order MemoryAnalysis::get_order()
{
	return BOTTOM_UP;
}

analysis_kind MemoryAnalysis::get_analysis_kind()
{
	return COMPASS_ANALYSIS;
}

sum_data_type MemoryAnalysis::get_summary_type()
{
	return COMPASS_SUMMARY;
}

void MemoryAnalysis::register_summary_callbacks()
{

}

Constraint MemoryAnalysis::get_current_stmt_guard()
{
	return stmt_guard;
}

void MemoryAnalysis::fill_blocks(sail::Block* cur, vector<sail::Block*> & to_fill,
		set<sail::Block*> & visited)
{
	if(visited.count(cur) > 0) return;
	if(cur == su->get_exit_block()){
		visited.insert(cur);
		to_fill.push_back(cur);
		return;
	}
	set<sail::CfgEdge*> & succs = cur->get_successors();
	set<sail::CfgEdge*, CompareBlockId> ordered_succs;
	ordered_succs.insert(succs.begin(), succs.end());
	set<sail::CfgEdge*, CompareBlockId>::iterator it = ordered_succs.begin();
	for(; it!= ordered_succs.end(); it++){
		sail::Block* b = (*it)->get_target();
		fill_blocks(b, to_fill, visited);
	}
	visited.insert(cur);
	to_fill.push_back(cur);
}

class CompareBlock:public binary_function<sail::Block*, sail::Block*, bool> {

public:
	bool operator()(const sail::Block* b1, const sail::Block* b2) const
	{
		return b1->get_block_id() < b2->get_block_id();
	}
};

void MemoryAnalysis::compute_loop_entry_block(sail::SuperBlock* su)
{
	set<sail::Block*> & body = su->get_body();
	set<sail::Block*, CompareBlock> ordered_body;
	ordered_body.insert(body.begin(), body.end());
	loop_entry_block = su->get_entry_block();
	set<sail::Block*, CompareBlock>::iterator it = ordered_body.begin();
	for(; it!= ordered_body.end(); it++)
	{
		sail::Block* cur = *it;
		set<sail::CfgEdge*> & succs = cur->get_successors();
		set<sail::CfgEdge*>::iterator it2 = succs.begin();
		bool exit = false;
		for(; it2!= succs.end(); it2++)
		{
			sail::Block* succ = (*it2)->get_target();
			if(succ->is_superblock()) continue;
			sail::BasicBlock* b = (sail::BasicBlock*)succ;
			if(su->is_exit_block(b))
			{
				exit = true;
				break;
			}
		}
		if(exit) {
			loop_entry_block = cur;
			break;
		}
	}


}

void MemoryAnalysis::compute_edge_constraints(sail::Block* b)
{

	assert_context("Computing edge constraints for block " +
			int_to_string(b->get_block_id()));
	bool is_loop_header = false;
	if(su->is_superblock())
	{
		is_loop_header =
			(loop_entry_block->get_block_id() == b->get_block_id());

	}
	set<sail::CfgEdge*, CompareBlockId> ordered_succs;
	ordered_succs.insert(b->get_successors().begin(), b->get_successors().end());
	set<sail::CfgEdge*, CompareBlockId>::iterator succs_it = ordered_succs.begin();

	map<sail::CfgEdge*, Constraint, CompareBlockId> cur_edge_constraints;
	for(; succs_it != ordered_succs.end(); succs_it++)
	{
		sail::CfgEdge* e = *succs_it;
		sail::Symbol* s = e->get_cond();

		if(dotties != NULL && s!= NULL && s->is_variable())
		{
			sail::Variable* sv = (sail::Variable*) s;
			if(sv->is_temp()) mg->remove_root(sv);
		}

		Constraint c;
		bool update = true;
		if(s==NULL) update = false;
		sail::Block* _target_block = e->get_target();

		if(is_loop_header)
		{

			sail::SuperBlock* sb = (sail::SuperBlock*)su;
			if(_target_block->is_basicblock())
			{
				sail::BasicBlock* target_block = (sail::BasicBlock*)_target_block;
				if(!sb->is_exit_block(target_block)) {
					//update = false;
				}
				else {
					c_assert(entry_cond.is_true());
					MemoryLocation* loc = mg->get_location(s);
					mg->ie.enforce_existence_and_uniqueness(loc, 0,
							Constraint(true), mg);
					entry_cond = !mg->get_neqz_constraint(s);
				}
			}
		}


		if(update) {
			MemoryLocation* loc = mg->get_location(s);
			assert(this->block_to_stmt_guard.count(b) > 0);


			mg->ie.enforce_existence_and_uniqueness(loc, 0,
					this->block_to_stmt_guard[b].end_c, mg);
			//Constraint(true), mg);
			c = mg->get_neqz_constraint(s);
		}
		/*
		 * If the current block is an exit block from a break point
		 * (i.e., it is not a direct child of the entry point),
		 * we need to and in the entry condition for the loop.
		 */
		if(su->is_superblock() && _target_block->is_basicblock())
		{
			sail::SuperBlock* sb = (sail::SuperBlock*)su;
			sail::BasicBlock* target_block = (sail::BasicBlock*)_target_block;
			if(sb->is_exit_block(target_block))
			{
				set<sail::CfgEdge*>& preds = target_block->get_predecessors();
				if(preds.size() == 1)
				{
					sail::CfgEdge* pred_edge = *preds.begin();

					if(pred_edge->get_source() != this->loop_entry_block)
					{
						c &= entry_cond;
					}
				}
			}
		}
		cur_edge_constraints[e] = c;

	}
	map<sail::CfgEdge*, Constraint, CompareBlockId>::iterator it =
			cur_edge_constraints.begin();

	bool precise = true;
	for(; it!= cur_edge_constraints.end(); it++)
	{
		if(!it->second.is_precise()) {
			precise = false;
			break;
		}
	}

	it = cur_edge_constraints.begin();
	if(precise)
	{
		for(; it!= cur_edge_constraints.end(); it++)
		{
			edge_constraints[it->first] = it->second;
		}
	}
	else
	{
		assert_context("Imprecise statement guard from block " +
				int_to_string(b->get_block_id()));
		c_assert(false);
	}

}


sail::SummaryUnit* MemoryAnalysis::get_su()
{
	return su;
}

/*
 * Substitutes old_ap with new_ap in statement guards.
 * This might be necessary when certain access paths are upgraded to
 * array pointers rather than just pointers.
 */
void MemoryAnalysis::update_statement_guards(Term* old_ap,
		Term* new_ap)
{
	stmt_guard.replace_term(old_ap, new_ap);
	map<sail::CfgEdge*, Constraint>::iterator it =edge_constraints.begin();
	for(; it!= edge_constraints.end(); it++)
	{
		Constraint& c = it->second;
		c.replace_term(old_ap, new_ap);

	}

	map<sail::Block*, block_constraint>::iterator it2 =
		block_to_stmt_guard.begin();
	for(; it2 != block_to_stmt_guard.end(); it2++)
	{
		block_constraint& bc = it2->second;
		bc.end_c.replace_term(old_ap, new_ap);
	}

	entry_cond.replace_term(old_ap, new_ap);


}

BlockExecutionCounter* MemoryAnalysis::get_bec()
{
	return bec;
}

int MemoryAnalysis::get_cur_iteration_counter()
{
	if(bec == NULL) return -1;
	return bec->get_iteration_counter(cur_block->get_block_id());
}

SummaryGraph* MemoryAnalysis::do_analysis(sail::SummaryUnit *su,
		bool report_errors, AnalysisResult *ar, SummaryFetcher * sf, int cg_id,
		map<il::type*, set<call_id> >* function_addresses, bool track_rtti)
{
	assert_context("Analyzing " + su->get_identifier().to_string());

	//cerr << "~~~~~~~~~~~~~~~~ STARTING " << su->get_identifier().to_string() << endl;
	int start = clock();
	/*int k, l;
	for(k=0; k<1000000000; k++)
	{
		l = l*3+k;
	}
	cout << l << endl;
	*/
	SummaryGraph* res = do_analysis(su, report_errors, sf, ar, NULL,  cg_id,
			function_addresses, track_rtti);


	int time = clock() - start;

	ar->time = ((double) time)/((double)CLOCKS_PER_SEC);
	ar->constraint_stats = Constraint::cs.stats_to_string();
	//cerr << "~~~~~~~~~~~~~~~~ ENDING " << su->get_identifier().to_string() << endl;
	return res;
}


void MemoryAnalysis::add_stack_layout_assumptions()
{
	sail::Function* f = NULL;
	if(su->is_function()) f = static_cast<sail::Function*>(su);
	else {
		sail::SuperBlock* sb = static_cast<sail::SuperBlock*>(su);
		f = sb->get_parent_function();
	}
	set<AccessPath*> address_vars;
	vector<sail::Instruction*>* inst = f->get_body();
	for(unsigned int i=0; i < inst->size(); i++)
	{
		sail::Instruction* cur = (*inst)[i];
		if(cur->get_instruction_id() != sail::ADDRESS_VAR) continue;
		sail::AddressVar* a = (sail::AddressVar*) cur;
		sail::Symbol* s = a->get_rhs();
		if(s->is_constant()){
			continue;
		}
		sail::Variable* v = (sail::Variable*)s;
		if(!v->is_local()) {
			continue;
		}
		if(v->is_anonymous()) {
			continue;
		}
		AccessPath* var = mg->get_access_path_from_symbol(v);
		address_vars.insert(var);

	}

	sail::Variable* prev_var = NULL;
	int prev_size = -1;
	const list<sail::Variable*>& locals = f->get_local_vars();
	list<sail::Variable*>::const_iterator it = locals.begin();
	for(; it!= locals.end(); it++)
	{


		sail::Variable* v = *it;
		AccessPath* cur_var = mg->get_access_path_from_symbol(v);
		if(address_vars.count(cur_var) == 0) {
			continue;
		}


		if(prev_var != NULL) {
			AccessPath* prev_var_ap = mg->get_access_path_from_symbol(prev_var);
			AccessPath* prev_addr = Address::make(prev_var_ap);
			AccessPath* cur_addr = Address::make(cur_var);

			// prev_addr + prev_size <= cur_addr
			AccessPath* av = ArithmeticValue::make_plus(prev_addr,
					ConstantValue::make(prev_size));
			Constraint cur_c =
					ConstraintGenerator::get_leq_constraint(av, cur_addr);

			Constraint::set_background_knowledge(cur_c);



		}


		prev_var = v;
		il::type* t = v->get_type();
		prev_size = t->get_size()/8;

	}
}

SummaryGraph* MemoryAnalysis::do_analysis(sail::SummaryUnit *su,
		bool report_all_errors,
		SummaryFetcher * sf, AnalysisResult *ar,
		vector<dotty_info>* dotties, int cg_id,
		map<il::type*, set<call_id> >* function_addresses, bool track_rtti)
{


	this->track_rtti = track_rtti;
	this->cg_id = cg_id;
	Variable::set_su_id(cg_id);

	mg = new MemoryGraph(ar->errors, false, *this, report_all_errors);


	if(DEBUG)
		cout << "----- analyzing "  << su->get_identifier()<< endl;
	//AccessPath::clear();
	this->su = su;

	this->sf = sf;
	if(!su->is_function()) {
		mg->set_recursive();

	}
	if(su->is_superblock())
		bec = new BlockExecutionCounter((sail::SuperBlock*) su);
	mg->set_summary_unit(su);
	mg->set_fn_id(su->get_identifier().get_function_name());
	mg->set_file(su->get_identifier().get_file_id().to_string());
	mg->set_error_report_status(report_all_errors);
	this->dotties = dotties;
	this->function_addresses = function_addresses;
	vector<sail::Block*> blocks;
	set<sail::Block*> visited;
	sail::BasicBlock* entry = su->get_entry_block();
	fill_blocks(entry, blocks, visited);
	if(su->is_superblock())
		compute_loop_entry_block((sail::SuperBlock*)su);

	int old_c = 0;
	int i = 0;
	string old_string = mg->ea.to_string();
	while(true){
		int c = clock();
		this->push_dotty_level(string("Iteration ") + int_to_string(i+1));

		process_blocks(blocks);
		this->add_dotty("End Iteration", "");
		this->pop_dotty_level();
		int t  = clock()-c;
		double time = (double)t/ (double)CLOCKS_PER_SEC;
		cout << "Run: " << i+1 << " took " << time << endl;

		cout << mg->ea.to_string() << endl;
		int count = mg->ea.count();
		if(count == 0) break;
		//if(count == old_c) break;
		old_c = count;

		string new_string = mg->ea.to_string();
		if(old_string ==  new_string) break;
		old_string = new_string;


		//cout << "############### AP MAP ################: " << i << endl;
		//mg->print_ap_to_loc_map(false);
		//cout << "############### AP MAP END ################ " << endl;



		i++;
		if(i>5) {
			cerr << "******DID NOT FIX POINT****" << endl;
			break;
		}

		//cout << Constraint::background_knowledge_to_string() << endl;
	}

	//process_blocks(blocks);


	/*
	for(int i=blocks.size()-1; i>=0; i--)
	{
		sail::Block* b = blocks[i];

		cur_block = b;
		stmt_guard = get_stmt_guard(b, false);




		if(DEBUG) {
			cout << "Processing block: " << b->to_string() << endl;
			cout << "Stmt guard: " << stmt_guard.to_string() << endl;
		}


		if(b->is_basicblock())
		{
			process_basic_block((sail::BasicBlock*)b);
		}
		else {
			process_super_block((sail::SuperBlock*)b);
		}

		compute_edge_constraints(b);
		processed_blocks.insert(b);
	}
	*/

	Constraint non_exception_cond;
	if(su->get_exception_block()!= NULL &&
			su->get_exception_block()->get_block_id() != -1) {
		non_exception_cond = !get_stmt_guard(su->get_exception_block(), true);
	}

	Constraint loop_continuation_cond(true);
	if(su->is_superblock())
	{
		Constraint exit_guard = get_stmt_guard(su->get_exit_block(), true);
		loop_continuation_cond = !get_stmt_guard(su->get_exit_block(), true);
	}
	return_cond &= non_exception_cond.nc();







	SummaryGraph * sg;
	if(su->is_function())
		sg = new SummaryGraph(mg, return_cond,  (sail::Function*)su);
	else sg = new SummaryGraph(mg, return_cond,  loop_continuation_cond,
			(sail::SuperBlock*)su);

	if(report_all_errors) mg->report_errors();


	if(DEBUG)
		cout << "----- Finished MemoryAnalysis of " <<  su->get_identifier()
		<< endl;


	return sg;

}

void MemoryAnalysis::process_blocks(vector<sail::Block*> & blocks)
{
	entry_cond = Constraint(true);
	processed_blocks.clear();
	stmt_guard = Constraint(true);
	block_to_stmt_guard.clear();
	edge_constraints.clear();
	alloc_per_lines.clear();
	instruction_number = 0;
	mg->ea.clear_stored();
	mg->clear();

	add_stack_layout_assumptions();

	for(int i=blocks.size()-1; i>=0; i--)
	{
		sail::Block* b = blocks[i];

		cur_block = b;
		stmt_guard = get_stmt_guard(b, false);




		if(DEBUG) {
			cout << "Processing block: " << b->to_string() << endl;
			cout << "Stmt guard: " << stmt_guard.to_string() << endl;
		}


		if(b->is_basicblock())
		{
			process_basic_block((sail::BasicBlock*)b);
		}
		else {
			process_super_block((sail::SuperBlock*)b);
		}

		compute_edge_constraints(b);
		processed_blocks.insert(b);
	}
}

void MemoryAnalysis::process_init_functions(vector<SummaryGraph*>& init_summaries)
{
	for(unsigned int i=0; i< init_summaries.size(); i++)
	{
		SummaryGraph* sg = init_summaries[i];
		vector<sail::Symbol*> empty_args;
		Constraint true_c(true);
		mg->process_function_call(NULL, sg, NULL, &empty_args, true_c);
	}
}

int MemoryAnalysis::get_cur_block_id()
{
	return cur_block->get_block_id();
}

int MemoryAnalysis::get_cg_id()
{
	return cg_id;
}

Constraint MemoryAnalysis::get_stmt_guard(sail::Block* b, bool end)
{

	assert_context("Getting statement guard for block " +
			int_to_string(b->get_block_id()));

	Constraint res;

	if(block_to_stmt_guard.count(b) > 0) {
		if(end) res &= block_to_stmt_guard[b].end_c;
		else res &= block_to_stmt_guard[b].begin_c;
		return res;
	}

	if(b->get_predecessors().size()==0){
		struct block_constraint bc = {res, res};
		block_to_stmt_guard[b] = bc;
		return res;
	}

	/*
	 * If this block post-dominates a block that dominates it,
	 * then they have the same statement guard.
	 */
	sail::BasicBlock* bb;
	if(b->is_superblock()) {
		bb = ((sail::SuperBlock*)b)->get_entry_block();
	}
	else bb = (sail::BasicBlock*) b;
	set<sail::BasicBlock*>& doms = bb->get_dominators();
	set<sail::BasicBlock*>::iterator it = doms.begin();
	for(; it!= doms.end(); it++) {
		sail::BasicBlock* cur = *it;
		if(cur == bb) continue;
		set<sail::BasicBlock*>& cur_pdoms = cur->get_post_dominators();
		if(cur_pdoms.count(bb) == 0) continue;
		if(block_to_stmt_guard.count(cur) == 0){
			continue;
		}
		if(end) res &= block_to_stmt_guard[cur].end_c;
		else res &= block_to_stmt_guard[cur].begin_c;
		struct block_constraint bc = {res, res};
		block_to_stmt_guard[b] = bc;
		return res;

	}



	set<sail::CfgEdge*> & preds = b->get_predecessors();
	set<sail::CfgEdge*, CompareBlockId> ordered_preds;
	ordered_preds.insert(preds.begin(), preds.end());
	set<sail::CfgEdge*, CompareBlockId>::iterator preds_it = ordered_preds.begin();
	sail::Block* pred = (*preds_it)->get_source();
	Constraint pred_guard=  (block_to_stmt_guard[pred].end_c);
	c_assert(edge_constraints.count(*preds_it) > 0);
	Constraint edge_guard = edge_constraints[*preds_it];
	res &= pred_guard & edge_guard;
	preds_it++;
	for(; preds_it != ordered_preds.end(); preds_it++)
	{
		sail::Block* pred = (*preds_it)->get_source();
		Constraint pred_guard = (block_to_stmt_guard[pred].end_c);
		c_assert(edge_constraints.count(*preds_it) > 0);
		Constraint edge_guard = edge_constraints[*preds_it];
		Constraint cur = pred_guard & edge_guard;
		res = res | cur;
		res.sat();
		res.valid();
	}
	//res.sat();
	//res.valid();

	struct block_constraint bc = {res, res};
	block_to_stmt_guard[b] = bc;
	return res;



}

void MemoryAnalysis::process_assume_size(sail::AssumeSize* inst)
{
	sail::Symbol* _buf = inst->get_buffer();
	sail::Symbol* size = inst->get_size();
	if(!_buf->is_variable()) {
		cerr << "Unexpected argument to assume_size statement: " <<
		inst->to_string(true) << endl;
		return;
	}
	sail::Variable* buf = (sail::Variable*) _buf;
	mg->process_set_buffer_size(buf, size, stmt_guard);
}

void MemoryAnalysis::process_basic_block(sail::BasicBlock* b)
{
	push_dotty_level("Basic Block " + int_to_string(b->get_block_id()),
			"Statement Guard: " + stmt_guard.to_string());



	if(b == su->get_entry_block()) {
		vector<SummaryGraph*> init_summaries;
		sf->get_init_function_summaries(init_summaries);
		process_init_functions(init_summaries);

		if(su->is_function()) {
			sail::Function* f = (sail::Function*) su;
			il::type* ret_type = f->get_return_type();
			if(ret_type != NULL && !ret_type->is_void_type()) {
				sail::Variable* retvar = sail::Variable::get_return_variable(ret_type);
				mg->initialize_return_value(retvar);
			}
		}
	}

	for(unsigned int i=0; i < b->get_statements().size(); i++)
	{

		if(stmt_guard.unsat()) return; // rest of the block is unreachable...
		process_statement(b->get_statements()[i]);
	}
	pop_dotty_level();
}

void MemoryAnalysis::process_super_block(sail::SuperBlock* sb)
{

	push_dotty_level("Super Block " + int_to_string(sb->get_block_id()),
			"Statement Guard: " + stmt_guard.to_string());
	SummaryGraph *s = sf->get_summary(call_id(sb->get_identifier()));

	if(s==NULL){
		assert_context("No loop summary found: " +
				sb->get_identifier().to_string());
		c_warn(false);
		return;
	}
	mg->process_loop_invocation(s, stmt_guard);

	processed_instructions.push_back("Loop");

	pop_dotty_level();

}

void MemoryAnalysis::process_statement(sail::Instruction *inst)
{

	if( DEBUG)
	{
		cout << "=> Processing statement: " << inst->to_string() << endl;
		cout << "Statement guard: " << this->stmt_guard.to_string() << endl;
		cout << "Instruction id: " << inst->get_instruction_id() << endl;
	}



	mg->set_instruction_number(instruction_number++);
	mg->set_line(inst->line);
	processed_instructions.push_back(inst->to_string(false));

	assert_context("Processing statement: " + inst->to_string() + " at line: " +
			int_to_string(inst->line) + " in: " + su->get_identifier().to_string());

	{
		assert_context("Client analyses process instruction before");
		if(inst->get_instruction_id() != sail::FUNCTION_CALL)
			ClientAnalysis::cam.process_instruction_before(inst);
	}

	switch(inst->get_instruction_id())
	{

	case sail::ASSIGNMENT:
	{
		process_assignment((sail::Assignment*)inst);
		break;
	}
	case sail::FIELD_REF_READ:
	{
		process_field_ref_read((sail::FieldRefRead*)inst);
		break;
	}
	case sail::FIELD_REF_WRITE:
	{
		process_field_ref_write((sail::FieldRefWrite*) inst);
		break;
	}
	case sail::LOAD:
	{
		process_load((sail::Load*) inst);
		break;
	}
	case sail::STORE:
	{
		process_store((sail::Store*)inst);
		break;
	}
	case sail::ADDRESS_VAR:
	{
		process_address_var((sail::AddressVar*) inst);
		break;
	}
	case sail::UNOP:
	{
		process_unop((sail::Unop*) inst);
		break;
	}

	case sail::BINOP:
	{
		process_binop((sail::Binop*) inst);
		break;
	}

	case sail::CAST:
	{
		process_cast((sail::Cast*)inst);
		break;
	}

	case sail::ARRAY_REF_READ:
	{
		process_array_ref_read((sail::ArrayRefRead*)inst);
		break;
	}
	case sail::ARRAY_REF_WRITE:
	{
		process_array_ref_write((sail::ArrayRefWrite*)inst);
		break;
	}

	case sail::DROP_TEMPORARY:
	{
		break;
	}
	case sail::STATIC_ASSERT:
	{
		process_static_assert((sail::StaticAssert*) inst);
		break;
	}
	case sail::ASSUME:
	{
		process_assume((sail::Assume*) inst);
		break;
	}
	case sail::ASSUME_SIZE:
	{
		process_assume_size((sail::AssumeSize*) inst);
		break;
	}

	case sail::ADDRESS_STRING:
	{
		process_address_string((sail::AddressString*)inst);
		break;
	}



	case sail::SAIL_ASSEMBLY:
	{
		//cerr << "Cannot process assembly instruction: " <<
		//inst->to_string() << endl;
		break;
	}

	case sail::FUNCTION_CALL:
	{
		sail::FunctionCall* fc = (sail::FunctionCall*) inst;
		if(fc->is_allocator()) {
			process_memory_allocation(fc);
			break;
		}

		if(fc->is_deallocator()){
			process_memory_deallocation(fc);
			break;
		}


		process_function_call((sail::FunctionCall*)inst);
			break;
	}
	case sail::FUNCTION_POINTER_CALL:
	{
		process_function_pointer_call((sail::FunctionPointerCall*)inst);
		break;
	}
	case sail::LOOP_INVOCATION:
		break;
	case sail::ADDRESS_LABEL:
	{
		process_address_label((sail::AddressLabel*) inst);
		break;
	}
	case sail::INSTANCEOF:
	{
		process_instance_of((sail::InstanceOf*) inst);
		break;
	}


	default:
	{
		cerr << "Unexpected Statement " << endl;
		//c_assert(false);
	}




	}
	if(dotties!= NULL) {
		add_dotty(inst);

		set<sail::Symbol*> used_syms;
		inst->get_rhs_symbols(used_syms);

		if(inst->get_instruction_id() == sail::STORE)
		{
			sail::Store* s = (sail::Store*) inst;
			sail::Symbol* lhs = s->get_lhs();
			if(lhs->is_variable()) {
				sail::Variable* v = (sail::Variable*) lhs;
				if(v->is_temp()) used_syms.insert(v);
			}
		}
		if(inst->get_instruction_id() == sail::FIELD_REF_WRITE)
		{
			sail::FieldRefWrite* s = (sail::FieldRefWrite*) inst;
			sail::Symbol* lhs = s->get_lhs();
			if(lhs->is_variable()) {
				sail::Variable* v = (sail::Variable*) lhs;
				if(v->is_temp()) used_syms.insert(v);
			}
		}

		else if(inst->get_instruction_id() == sail::ARRAY_REF_WRITE)
		{
			sail::ArrayRefWrite* s = (sail::ArrayRefWrite*) inst;
			sail::Symbol* lhs = s->get_lhs();
			if(lhs->is_variable()) {
				sail::Variable* v = (sail::Variable*) lhs;
				if(v->is_temp()) used_syms.insert(v);
			}
			sail::Symbol* index = s->get_index();
			if(index->is_variable()) {
				sail::Variable* index_v = (sail::Variable*) index;
				if(index_v->is_temp()) used_syms.insert(index_v);
			}
		}

		set<sail::Symbol*>::iterator it = used_syms.begin();
		for(; it!= used_syms.end(); it++){
			sail::Symbol* s = *it;
			if(!s->is_variable()) continue;
			sail::Variable* sv = (sail::Variable*) s;
			if(sv->is_temp()) {
				mg->remove_root(sv);
			}
		}



	}

	{
		assert_context("Client analyses process instruction after");
		if(inst->get_instruction_id() != sail::FUNCTION_CALL)
			ClientAnalysis::cam.process_instruction_after(inst);
	}
}

dotty_info::dotty_info(int level, const string & key, const string & val1,
			const string & val2)
{
	this->level = level;
	this->key = key;
	this->val1 = val1;
	this->val2 = val2;
}

string MemoryAnalysis::escape_dotty_string(const string & s)
{
	string res;
	for(unsigned int i=0; i<s.size(); i++) {
		if(s[i]=='>'){
			res+="\\>";
			continue;
		}
		if(s[i]=='<'){
			res+="\\<";
			continue;
		}
		if(s[i]=='|'){
			res+="\\|";
			continue;
		}
		if(s[i]=='\n'){
			res+="\\n";
			continue;
		}
		res+=s[i];
	}
	return res;
}



string MemoryAnalysis::dotty_from_info(const string& info)
{
	string res;
	res = "digraph G { rankdir=LR;\n";
	res += "ranksep=1.2;\n";
	res += "nodesep=0.9;\n";
	res+=string("node_info") + " [shape = record]";
	res+="[port=e][height=0.1][label= \"";

	res+= "{{" + escape_dotty_string(info);
	res+="}}";
	res+= "\"]\n";
	res += "}\n";
	return res;
}

void MemoryAnalysis::push_dotty_level(const string & section,
		const string & info)
{

	if(dotties == NULL) return;
	string dotty = dotty_from_info(info);
	dotties->push_back(dotty_info(dotty_level, section, dotty, dotty));
	this->dotty_level++;
}

void MemoryAnalysis::pop_dotty_level()
{
	if(dotties == NULL) return;
	this->dotty_level--;

}

MemoryGraph* MemoryAnalysis::get_mg()
{
	return this->mg;
}

void MemoryAnalysis::add_dotty(sail::Instruction *inst)
{
	if(dotties == NULL) return;
	set<sail::Symbol*> affected_syms;
	inst->get_symbols(affected_syms);

	set<AccessPath*> aps;
	set<sail::Symbol*>::iterator it = affected_syms.begin();
	for(; it != affected_syms.end(); it++)
	{
		if(*it == NULL) continue;
		AccessPath* ap = mg->get_access_path_from_symbol(*it);
		aps.insert(ap);
	}


	dotties->push_back(dotty_info(dotty_level, inst->to_string(false),
			mg->to_dotty(aps), mg->to_dotty()));
}

void MemoryAnalysis::add_dotty(const string & label, set<AccessPath*>& aps)
{
	if(dotties == NULL) return;
	dotties->push_back(dotty_info(dotty_level, label,
			mg->to_dotty(aps),mg->to_dotty()));
}


void MemoryAnalysis::add_dotty(const string & label, AccessPath* ap1)
{
	set<AccessPath*> aps;
	aps.insert(ap1);
	add_dotty(label, aps);
}
void MemoryAnalysis::add_dotty(const string & label, AccessPath* ap1,
		AccessPath* ap2)
{
	set<AccessPath*> aps;
	aps.insert(ap1);
	aps.insert(ap2);
	add_dotty(label, aps);
}
void MemoryAnalysis::add_dotty(const string & label, AccessPath* ap1,
		AccessPath* ap2, AccessPath* ap3)
{
	set<AccessPath*> aps;
	aps.insert(ap1);
	aps.insert(ap2);
	aps.insert(ap3);
	add_dotty(label, aps);
}

void MemoryAnalysis::add_dotty(const string & label, const string & dotty)
{
	if(dotties == NULL) return;
	dotties->push_back(dotty_info(dotty_level, label,
			dotty, dotty));
}



void MemoryAnalysis::process_function_call(sail::FunctionCall* inst)
{
	if(inst->is_virtual_call()) {

		process_virtual_method_call(inst);
		return;

	}
	if(inst->get_function_name() == OBSERVED &&
				inst->get_arguments()->size() == 1)
		{
			sail::Symbol* buf = *inst->get_arguments()->begin();
			mg->process_observed(buf, stmt_guard);
			return;
		}
	if(inst->get_function_name() == BUFFER_SIZE &&
			inst->get_arguments()->size() == 1)
	{
		sail::Variable* res = inst->get_return_variable();
		if(res == NULL) return;
		sail::Symbol* buf = *inst->get_arguments()->begin();
		mg->process_get_buffer_size(res, buf, stmt_guard);
		return;
	}

	if(inst->get_function_name() == BUFFER_SAFE &&
			inst->get_arguments()->size() == 2)
	{

		sail::Symbol* buf = (*inst->get_arguments())[0];
		sail::Symbol* offset = (*inst->get_arguments())[1];
		if(buf->is_constant())
		{
			cerr << "Check of buffer size on constant not possible" << endl;
			return;
		}
		mg->check_buffer_access((sail::Variable*)buf, offset, stmt_guard);
		return;
	}
	if(inst->get_function_name() == ASSIGN &&
				inst->get_arguments()->size() == 4)
	{
		sail::Symbol* source = (*inst->get_arguments())[0];
		sail::Symbol* target = (*inst->get_arguments())[1];
		sail::Symbol* nc = (*inst->get_arguments())[2];
		sail::Symbol* sc = (*inst->get_arguments())[3];
		mg->process_assign_function(source, target, nc, sc, stmt_guard);
		return;
	}
	if(inst->get_function_name() == NIL_FN)
	{
		sail::Variable* ret = inst->get_return_variable();
		if(ret == NULL) return;
		Variable* v = (Variable*)mg->get_access_path_from_symbol(ret);
		mg->process_assignment(v, Nil::make(), stmt_guard);
		return;

	}
	if(inst->get_function_name() == IS_NIL_FN)
	{
		sail::Variable* ret = inst->get_return_variable();
		if(ret == NULL) return;
		if(inst->get_arguments()->size() == 1)
		{
			Variable* res = (Variable*)mg->get_access_path_from_symbol(ret);
			MemoryLocation* loc = mg->get_location(res);
			mg->add_default_edges(loc);
			sail::Symbol* arg = (*inst->get_arguments())[0];
			AccessPath* arg_ap = mg->get_access_path_from_symbol(arg);
			set<pair<AccessPath*, Constraint> > values;
			mg->get_value_set(arg_ap, values);
			Constraint is_nil_c(false);
			set<pair<AccessPath*, Constraint> >::iterator it = values.begin();
			for(; it!= values.end(); it++) {
				AccessPath* cur_val = it->first;
				Constraint cur_c = it->second;
				AccessPath* is_nil_fn = FunctionValue::make_is_nil(cur_val);

				Constraint cur = (cur_c &
						ConstraintGenerator::get_neqz_constraint(is_nil_fn));
				IndexVarManager::eliminate_source_vars(cur);
				is_nil_c |= cur;

			}
			mg->update_outgoing_edges(loc, stmt_guard);
			mg->put_edge(res, ConstantValue::make(1)->add_deref(),
					is_nil_c & stmt_guard, false, false);

			mg->put_edge(res, ConstantValue::make(0)->add_deref(),
					(!is_nil_c) & stmt_guard, false, false);
			return;

		}


	}


	if(inst->is_exit_function())
	{
		update_statement_guard(Constraint(false));
		return;
	}

	if(inst->get_function_name() == ADT_INSERT)
	{
		vector<sail::Symbol*>* args = inst->get_arguments();
		if(args->size() != 3) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Wrong number of "
					"arguments to ADT_INSERT function");
			return;
		}
		sail::Symbol* adt = (*args)[0];
		if(!adt->is_variable()) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "First argument to "
					"ADT_INSERT must be a variable.");
			return;
		}

		sail::Symbol* key = (*args)[1];
		sail::Symbol* value = (*args)[2];



		 string name = "t_mem" + int_to_string(instruction_number);
		 il::type* t= value->get_type()->get_inner_type();

		if(t->is_abstract_data_type()) {
			il::type* value_t = t->get_adt_value_type();
			t = il::pointer_type::make(value_t,"");
		}
		sail::Variable* temp = new sail::Variable(name,
						t, true);
		string pp = "*" + value->to_string(true);
		mg->process_load(temp, (sail::Variable*)value, 0,
				stmt_guard);





		mg->process_adt_insert((sail::Variable*) adt, key, temp, stmt_guard);
		//mg->process_adt_insert((sail::Variable*) adt, key, value, stmt_guard);
		return;

	}

	if(inst->get_function_name() == ADT_READ ||
			inst->get_function_name() == ADT_READ_POS)
	{
		// if the return value is not captured, this is a noop.
		if(inst->get_return_variable() == NULL) return;

		vector<sail::Symbol*>* args = inst->get_arguments();
		if(args->size() != 2) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Wrong number of "
					"arguments to ADT_READ function");
			return;
		}

		sail::Variable* res = inst->get_return_variable();


		sail::Symbol* adt = (*args)[0];
		if(!adt->is_variable()) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Second argument to "
					"ADT_READ must be a variable.");
			return;
		}


		sail::Symbol* key = (*args)[1];
		mg->process_adt_read((sail::Variable*) res, (sail::Variable*) adt,
				key, stmt_guard, (inst->get_function_name() == ADT_READ));
		return;
	}


	if(inst->get_function_name() == ADT_READ_REF ||
			inst->get_function_name() == ADT_READ_POS_REF)
	{
		// if the return value is not captured, this is a noop.
		if(inst->get_return_variable() == NULL) return;

		vector<sail::Symbol*>* args = inst->get_arguments();
		if(args->size() != 2) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Wrong number of "
					"arguments to ADT_READ function");
			return;
		}

		sail::Variable* res = inst->get_return_variable();


		sail::Symbol* adt = (*args)[0];
		if(!adt->is_variable()) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Second argument to "
					"ADT_READ must be a variable.");
			return;
		}


		sail::Symbol* key = (*args)[1];
		mg->process_adt_read_ref((sail::Variable*) res, (sail::Variable*) adt,
				key, stmt_guard, inst->get_function_name() == ADT_READ_REF);
		return;
	}

	if(inst->get_function_name() == ADT_GET_SIZE)
	{
		// if the return value is not captured, this is a noop.
		if(inst->get_return_variable() == NULL) return;

		if(inst->get_arguments()->size() != 1) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Wrong number of "
					"arguments to ADT_GET_SIZE function");
			il::type* sig = inst->get_signature();
			c_assert(sig->is_function_type());
			il::type* return_type = ((il::function_type*) sig)->get_return_type();
			mg->process_unknown_function_call(inst->get_return_variable(),
						inst->get_arguments(), inst->get_function_name(), stmt_guard,
						return_type);
			return;
		}

		sail::Symbol* adt = *inst->get_arguments()->begin();
		if(!adt->is_variable()) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "First argument to "
					"ADT_GET_SIZE must be a variable.");
		}
		mg->process_adt_get_size(inst->get_return_variable(),
				(sail::Variable*)adt, stmt_guard);
		return;

	}

	if(inst->get_function_name() == ADT_SET_SIZE)
	{
		if(inst->get_arguments()->size() != 2) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Wrong number of "
					"arguments to ADT_SET_SIZE function");
			return;
		}

		vector<sail::Symbol*>* args = inst->get_arguments();
		sail::Symbol* adt = (*args)[0];
		if(!adt->is_variable()) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "First argument to "
					"ADT_SET_SIZE must be a variable.");
			return;
		}
		mg->process_adt_set_size((sail::Variable*) adt, (*args)[1], stmt_guard);
		return;

	}



	if(inst->get_function_name() == ADT_REMOVE)
	{
		vector<sail::Symbol*>* args = inst->get_arguments();
		if(args->size() != 2) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Wrong number of "
					"arguments to ADT_REMOVE function");
			return;
		}

		sail::Symbol* adt = (*args)[0];
		if(!adt->is_variable()) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "First argument to "
					"ADT_REMOVE must be a variable.");
			return;
		}

		mg->process_adt_remove((sail::Variable*) adt, (*args)[1], stmt_guard);
		return;

	}

	if(inst->get_function_name() == ADT_CONTAINS)
	{
		// if the return value is not captured, this is a noop.
		if(inst->get_return_variable() == NULL) return;

		vector<sail::Symbol*>* args = inst->get_arguments();
		if(args->size() != 2) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Wrong number of "
					"arguments to ADT_COUNT function");
			return;
		}

		sail::Symbol* adt = (*args)[0];
		if(!adt->is_variable()) {
			mg->report_error(ERROR_ILLEGAL_ADT_FUNCTION, "First argument to "
					"ADT_COUNT must be a variable.");
			return;
		}

		mg->process_adt_contains(inst->get_return_variable(),
				(sail::Variable*) adt, (*args)[1],stmt_guard);
		return;
	}

	if(inst->get_function_name() == BUILTIN_MEMCPY)
	{
		vector<sail::Symbol*>* args = inst->get_arguments();
		c_assert(args->size() >= 2);
		sail::Symbol* lhs = (*args)[0];
		assert(lhs->is_variable());
		sail::Symbol* rhs = (*args)[1];
		assert(rhs->is_variable());
		string name = "t_mem" + int_to_string(instruction_number);

		il::type* t = rhs->get_type()->get_inner_type();
		if(t->is_abstract_data_type()) {
					il::type* value_t = t->get_adt_value_type();
					t = il::pointer_type::make(value_t,"");
				}

		sail::Variable* temp = new sail::Variable(name,
				t, true);
		mg->process_load(temp, (sail::Variable*) rhs, 0, stmt_guard);

		set<AccessPath*> aps;
		aps.insert(mg->get_access_path_from_symbol(temp));
		aps.insert(mg->get_access_path_from_symbol(rhs));
		add_dotty("Builtin memcpy load", aps);

		mg->process_store((sail::Variable*) lhs, 0, temp, stmt_guard);
		sail::Variable* retvar = inst->get_return_variable();
		if(retvar != NULL) {
			mg->process_assignment(retvar, lhs, stmt_guard);
		}
		return;
	}


	call_id ci(inst->get_function_name(), inst->get_namespace(),
			inst->get_signature(), su->get_identifier().get_language());


	SummaryGraph *s = sf->get_summary(ci);


	if(s!= NULL) {

		{
			assert_context("Client analyses process instruction before");
			ClientAnalysis::cam.process_function_call_before(inst, true);
		}
		mg->process_function_call(inst, s, inst->get_return_variable(),
					inst->get_arguments(), stmt_guard);
		{
			assert_context("Client analyses process instruction before");
			ClientAnalysis::cam.process_function_call_after(inst, true);
		}
	}
	else
	{
		{
			assert_context("Client analyses process instruction before");
			ClientAnalysis::cam.process_function_call_before(inst, false);
		}

		if(inst->is_constructor()) {
			mg->process_constructor_call(inst, inst->get_arguments(), stmt_guard);
		}

		il::type* sig = inst->get_signature();
		c_assert(sig->is_function_type());
		il::type* return_type = ((il::function_type*) sig)->get_return_type();
		mg->process_unknown_function_call(inst->get_return_variable(),
			inst->get_arguments(), inst->get_function_name(), stmt_guard,
			return_type);

		{
			assert_context("Client analyses process instruction after");
			ClientAnalysis::cam.process_function_call_after(inst, false);
		}
	}




}

void MemoryAnalysis::update_statement_guard(Constraint c)
{


	Constraint res = (c | !stmt_guard);;

	Constraint::set_background_knowledge(res);
	return_cond &= res.nc();



}

void MemoryAnalysis::process_virtual_method_call(sail::FunctionCall* inst)
{

	set<call_id> targets;
	inst->get_virtual_call_targets(targets);
	set<SummaryGraph*> target_summaries;
	set<call_id>::iterator it = targets.begin();
	for( ; it!= targets.end(); it++)
	{
		call_id cid = *it;
		SummaryGraph *s = sf->get_summary(cid);
		if(s==NULL) continue;

		target_summaries.insert(s);

	}


	// Find the dispatch pointer
	vector<sail::Symbol*>* args = inst->get_arguments();
	c_assert(args->size() > 0);
	sail::Symbol* _dispatch_ptr= (*args)[0];
	c_assert(_dispatch_ptr->is_variable());
	sail::Variable* dispatch_ptr = (sail::Variable*) _dispatch_ptr;

	sail::Variable* ret_var = inst->get_return_variable();
	//cout << "*********** procesding call **********" << inst->to_string() << endl;
//	cout << "type: " <<dispatch_ptr->get_type()->to_string() << endl;
	{
		assert_context("Client analyses process instruction before");
		ClientAnalysis::cam.process_function_call_before(inst,
				target_summaries.size()!=0);
	}


	mg->process_virtual_method_call(dispatch_ptr, target_summaries, ret_var,
			args, stmt_guard);

	{
			assert_context("Client analyses process instruction before");
			ClientAnalysis::cam.process_function_call_after(inst,
					target_summaries.size()!=0);
	}
}

void MemoryAnalysis::process_function_pointer_call(sail::FunctionPointerCall* inst)
{



	il::type* ret_type = (inst->get_return_variable() == NULL ? NULL :
		inst->get_return_variable()->get_type());


	il::type* signature = inst->get_function_pointer()->get_type()->get_deref_type();

	if(this->function_addresses->count(signature) == 0) {
		cout << "No matching signature: " << signature->to_string() << endl;
		mg->process_unknown_function_call(inst->get_return_variable(),inst->get_arguments(),
				"fn_ptr"+ int_to_string(instruction_number),stmt_guard,
				ret_type);
	}

	else {
		set<call_id>& call_ids = (*function_addresses)[signature];
		set<call_id>::iterator it = call_ids.begin();
		set<SummaryGraph*> target_summaries;
		for(; it!= call_ids.end(); it++)
		{
			call_id ci = *it;
			SummaryGraph *s = sf->get_summary(ci);
			if(s == NULL){
				continue;
			}
			target_summaries.insert(s);
		}

		if(target_summaries.size() > 0) {
			mg->process_function_pointer_call(inst->get_function_pointer(),
					target_summaries,
					inst->get_return_variable(),
						inst->get_arguments(), stmt_guard);
		}
		else
		{
			mg->report_error(ERROR_FUNCTION_POINTER_WITH_NO_TARGET, "");
		}

	}



	/*

	set<string> & targets = inst->get_targets();
	set<string>::iterator it = targets.begin();
	for(; it!= targets.end(); it++) {

		string key = *it;
		SummaryGraph *s = sf->get_summary(key);
		if(s == NULL) continue;
		mg->process_function_call(s, inst->get_return_variable(),
				inst->get_arguments(), *it, stmt_guard);
	}
	*/


}

void MemoryAnalysis::process_address_label(sail::AddressLabel* inst)
{
	mg->process_address_label(inst->get_lhs(), inst->get_label(),
			inst->get_signature(), stmt_guard);
}

void MemoryAnalysis::process_instance_of(sail::InstanceOf* inst)
{
	mg->process_instance_of(inst->get_lhs(), inst->get_rhs(),
				inst->get_instance_type(), stmt_guard);
}

void MemoryAnalysis::process_address_string(sail::AddressString* as)
{
	mg->process_address_string(as->get_lhs(), as->get_string(), stmt_guard);
}

void MemoryAnalysis::process_cast(sail::Cast* inst)
{
	sail::Variable* lhs = inst->get_lhs();
	sail::Symbol* rhs = inst->get_rhs();
	mg->process_cast(lhs, rhs, stmt_guard);
}

vector<string> & MemoryAnalysis::get_processed_instructions()
{
	return processed_instructions;
}



void MemoryAnalysis::process_assignment(sail::Assignment *inst)
{

	mg->process_assignment(inst->get_lhs(), inst->get_rhs(), stmt_guard);

}



void MemoryAnalysis::process_static_assert(sail::StaticAssert* inst)
{
	string loc;
	if(inst->get_original_node() != NULL)
		loc = inst->get_original_node()->get_location().to_string(true);
	else loc = inst->line;

	mg->process_static_assert(inst->get_predicate(), stmt_guard, loc);
}

void MemoryAnalysis::process_assume(sail::Assume* inst)
{
	sail::Symbol* pred = inst->get_predicate();
	Constraint assumption = mg->get_neqz_constraint(pred);
	update_statement_guard(assumption);

}

void MemoryAnalysis::process_memory_deallocation(sail::FunctionCall* fc)
{
	vector<sail::Symbol*>* args = fc->get_arguments();
	assert(args->size() >=1);
	sail::Symbol* ptr = *args->begin();
	assert(ptr->is_variable());
	mg->process_memory_deallocation((sail::Variable*)ptr, stmt_guard);

}

void MemoryAnalysis::process_memory_allocation(sail::FunctionCall* fc)
{
	sail::Variable* v = fc->get_lhs();
	sail::Symbol* alloc_size;

	if(fc->get_arguments()->size() > 0)
		alloc_size = (*fc->get_arguments())[0];
	else
	{
		if(fc->get_lhs() == NULL) alloc_size = new sail::Constant(0, true, 4);
		else
		{
			il::type* lhs_type = fc->get_lhs()->get_type();
			int size = lhs_type->get_size()/8;
			alloc_size = new sail::Constant(size, true, 4);
		}
	}

	string alloc_id = su->get_identifier().to_string();
	int line = fc->line;
	alloc_id += ":" + int_to_string(line);
	if(alloc_per_lines.count(line) == 0){
		alloc_per_lines[line] = 1;
	}
	else {
		int c = alloc_per_lines[line];
		alloc_per_lines[line] = c+1;
		alloc_id+=":" + int_to_string(c);
	}



	mg->process_memory_allocation(v, alloc_id, alloc_size, stmt_guard,
			fc->is_operator_new());

}

void MemoryAnalysis::process_field_ref_read(sail::FieldRefRead *inst)
{
	mg->process_assignment(inst->get_lhs(), inst->get_rhs(),inst->get_offset()/8
			,stmt_guard);
}

void MemoryAnalysis::process_field_ref_write(sail::FieldRefWrite *inst)
{
	mg->process_assignment(inst->get_lhs(),inst->get_offset()/8
			, inst->get_rhs(),stmt_guard);
}

void MemoryAnalysis::process_array_ref_read(sail::ArrayRefRead* inst)
{
	mg->process_array_ref_read(inst->get_lhs(), inst->get_rhs(), inst->get_index(),
			inst->get_offset()/8, stmt_guard);
}

void MemoryAnalysis::process_array_ref_write(sail::ArrayRefWrite* inst)
{
	mg->process_array_ref_write(inst->get_lhs(), inst->get_index(),
			inst->get_offset()/8, inst->get_rhs(), stmt_guard);
}

void MemoryAnalysis::process_load(sail::Load *inst)
{
	sail::Symbol* rhs = inst->get_rhs();
	if(!rhs->is_variable()){
		cerr << "Encountered illegal load instruction: " <<
			inst->to_string(true) << endl;
		return;
	}
	sail::Variable* rhs_var = (sail::Variable*) rhs;

	mg->process_load(inst->get_lhs(), rhs_var, inst->get_offset()/8, stmt_guard);
}

void MemoryAnalysis::process_store(sail::Store* inst)
{
	sail::Symbol* lhs = inst->get_lhs();
	if(!lhs->is_variable()){
		cerr << "Encountered illegal store instruction: " <<
			inst->to_string(true) << endl;
		return;
	}
	sail::Variable* lhs_var = (sail::Variable*) lhs;
	sail::Symbol* rhs = inst->get_rhs();
	mg->process_store(lhs_var, inst->get_offset()/8, rhs, stmt_guard);
}

void MemoryAnalysis::process_address_var(sail::AddressVar* inst)
{
	sail::Variable* lhs = inst->get_lhs();
	sail::Symbol* rhs = inst->get_rhs();
	mg->process_address(lhs, rhs, stmt_guard);
}

void MemoryAnalysis::process_unop(sail::Unop* inst)
{
	sail::Variable* var = inst->get_lhs();
	sail::Symbol* s= inst->get_operand();
	switch(inst->get_unop())
	{
	case il::_NEGATE:
	{
		mg->process_negate(var, s, stmt_guard);
		break;
	}
	case il::_BITWISE_NOT:
	{
		mg->process_bitwise_not(var, s, stmt_guard);
		break;
	}
	case il::_LOGICAL_NOT:
	{
		mg->process_logical_not(var,s ,stmt_guard);
		break;
	}
	default:
	{
		c_assert(false);
	}
	}
}

void MemoryAnalysis::process_binop(sail::Binop* inst)
{
	sail::Variable* lhs = inst->get_lhs();
	sail::Symbol* op1 = inst->get_first_operand();
	sail::Symbol* op2 = inst->get_second_operand();
	il::binop_type bt = inst->get_binop();
	switch(bt)
	{
		case il::_PLUS:
		case il::_POINTER_PLUS:
		{
			mg->process_plus(lhs, op1, op2, stmt_guard);
			break;
		}
		default:
		{
			if(il::binop_expression::is_predicate_binop(bt)){
				mg->process_predicate_binop(lhs, op1, op2, stmt_guard, bt);
			}
			else {
				mg->process_binop(lhs, op1, op2, stmt_guard, bt);
			}

		}
	}
}

MemoryAnalysis::~MemoryAnalysis() {

	ClientAnalysis::cam.register_memory_analysis(NULL);

}


bool CompareBlockId::operator()(const sail::CfgEdge* _s1,
		const sail::CfgEdge* _s2) const
{
	sail::CfgEdge* s1 = (sail::CfgEdge*)_s1;
	sail::CfgEdge* s2 = (sail::CfgEdge*)_s2;
	if(s1->get_source()->get_block_id() < s2->get_source()->get_block_id())
		return true;
	if(s1->get_source()->get_block_id() > s2->get_source()->get_block_id())
		return false;
	if(s1->get_target()->get_block_id() != s2->get_target()->get_block_id())
	{
		return s1->get_target()->get_block_id() <
		s2->get_target()->get_block_id();
	}
	return s1->get_cond()->to_string(false) < s2->get_cond()->to_string(false);
}
