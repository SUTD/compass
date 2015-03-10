#include "MtdInstsProcessor.h"
#include "SummaryUnit.h"
#include "StaticAssertBuilder.h"
#include "CallManager.h"
#include "AllocContextController.h"
#include "MemNode.h"
#include "SideNode.h"

#define ANALYZE_INS true
#define CLEAR_DOT_FILE true
#define WRITE_TO_DOT_FILES false
#define OUT_EDGE_WIDENING 10
#define CHECK_TIME 5
#define WRITE_TO_TEST_FILE true



using namespace std;

CallManager* MtdInstsProcessor::callm = NULL;

MtdInstsProcessor::MtdInstsProcessor(const unsigned int& idx,
		const vector<sail::Function*>& funs, const string& output_f) {

	i = idx;
	output_folder = output_f;
	f = funs.at(i);

	a_builder = new StaticAssertBuilder(this);
	assert(a_builder != NULL);

	mem_graph = new MemGraph();

	summary_unit_init();

	has_alloc = false;
	allocc = NULL;


	//make sure each method has one ele
	//directly use it;
#ifdef TERMINAL_SIDES_OPEN
		SideNode::sidenodes.clear();
		SideNode::unique_sides.clear();
#endif

}

void MtdInstsProcessor::summary_unit_init(){
	summary_unit = new SummaryUnit(this);

//	if((f->get_return_t() != NULL) &&
//			(!(f->get_return_t()->is_void_type())))

	if(f->get_return_t() != NULL){
		assert(!f->get_return_t()->is_void_type());
		summary_unit->set_return_var_flag(true);
	}
}



void MtdInstsProcessor::reset_summary_unit(){
	cerr << "call reset summary_unit for " <<
			this->f->get_identifier().to_string() << endl;

	delete summary_unit;
	summary_unit = NULL;

	summary_unit_init();
	this->alloc_mem_pointed_by_var.clear();

	summary_unit->build_equivalent_insts(*mem_graph);
}

#ifdef TERMINAL_SIDE_OPEN
const void MtdInstsProcessor::print_terminal_sides() const{
	cout << "		Print terminal sides !, should only have pointed by!! ";
	cout << "		terminal size is " << terminal_sides.size() << endl;
	int i = 0;
	for(auto it = terminal_sides.begin(); it != terminal_sides.end(); it++ ){
		if((*it)->has_point_to()){
			cerr << "Terminal_side shouldn't have point to element ! " << endl;
			assert(false);
		}
		cout << i ++ << " " << (*it)->to_string() << endl << endl;
	}

	cout << " 		!!!!!!!!!!!!!!!! end of Print terminal sides! " << endl << endl<< endl;
}
#endif



MtdInstsProcessor::~MtdInstsProcessor() {
}

bool MtdInstsProcessor::is_pointed_by_var(MemNode* deref_alloc_mn){
	return (alloc_mem_pointed_by_var.count(deref_alloc_mn) > 0);
}


void MtdInstsProcessor::put_to_alloc_pointed_by_var_map(
		MemNode* deref_alloc_mn, sail::Variable* point_to){
	assert(deref_alloc_mn != NULL);
	assert(point_to != NULL);
	alloc_mem_pointed_by_var[deref_alloc_mn]= point_to;
}



sail::Variable* MtdInstsProcessor::get_point_to_alloc_var(MemNode* deref_alloc_mn){

	assert(deref_alloc_mn != NULL);
	cout << "deref of alloc nodes is " <<deref_alloc_mn->to_string() << endl;
	//assert(alloc_mem_pointed_by_var.count(deref_alloc_mn) >0);
	if(alloc_mem_pointed_by_var.count(deref_alloc_mn) > 0)
		return alloc_mem_pointed_by_var[deref_alloc_mn];
	return NULL;

}


string MtdInstsProcessor::alloc_mem_pointed_by_var_to_string(){
	string str;

	map<MemNode*, sail::Variable*>::iterator it = alloc_mem_pointed_by_var.begin();
	for(; it != alloc_mem_pointed_by_var.end(); it++){
		str += "MemNode : ";
		str += it->first->to_string();
		str += "\n";
		str += "VAR : ";
		str += it->second->to_string();
		str += "\n";
	}

	return str;
}
void MtdInstsProcessor::process_basic_instructions() {
	cout << "1) inside process_basic_instructions() !!! " << endl;
	cout << "Function is " << f->to_string() << endl;
//	if (CLEAR_DOT_FILE) {
//		string cmd = "rm " + output_folder + "/*";
//		system(cmd.c_str());
//	}


	if (PROCESS_STAITC_ASSERT)
		set_up_complete_static_assert_eles();


	cout << "total instruction inside the function !! "
			<< f->get_body()->size()<<endl;


	if(f->is_empty())
		return;

	for (unsigned int i = 0; i < f->get_body()->size(); i++) {

		sail::Instruction* ins = f->get_body()->at(i);
//		if(i == 923){
//			cout << "function is " << f->get_identifier().to_string() << endl;
//			exit(1);
//		}
		assert(ins != NULL);

		if (0) {
			cout << "inst === " << ins->to_string() << "  ";
			if ((ins->get_instruction_id() != sail::BRANCH)
					&& (ins->get_instruction_id() != sail::SAIL_LABEL)
					&& (ins->get_instruction_id() != sail::JUMP))
				cout << "BID: [" << ins->get_basic_block()->get_block_id()
						<< "]" << endl << endl;
			else
				cout << endl;
		}


		process_first_round_single_instruction(f, ins, i, output_folder);
		//check if it needs to be collected;


	}


	if(0)
		cout << "Testing collected rules ===  " << endl;
			cout << rule_to_string() << " -- - -- end of rule_to_string()! " <<endl;

	process_instructions_in_rule();



	//////////////////////////
	//build summary-unit

	//if (WRITE_TO_DOT_FILES)
	write_to_dotfiles(f->get_identifier(), "first_round",
				output_folder, mem_graph);

	summary_unit->build_equivalent_insts(*mem_graph);




	//set it to the call-manager map
	//init unresolved_callid_summary and call_id_mtdinstsProcessor
	call_manager_map_init();



	//testing collected functioncalls
	if (0) {
		cout << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHH  " << endl;
		unresolved_functioncall_test();
	}

}

unsigned int MtdInstsProcessor::get_method_idx() const{
	return i;
}

void MtdInstsProcessor::call_manager_map_init(){

	call_id ci = identifier_to_call_id(f->get_identifier());
	assert(summary_unit != NULL);

	assert(callm != NULL);

	cout << endl << endl <<"###########################################" << endl;
	cout << "SUM stored to map::  "  <<summary_unit->to_string() <<
			"  for   " << ci.to_string() <<endl;

	//it is possible that the method's function signature has been changed to dummy;
	if(callm->is_dummy_id(this->i)){

		call_id& dummy_id = callm->get_dummy_id(this->i);
		ci = dummy_id;
		cout << "GGGeting dummy id" << ci.to_string() << endl;
	}

	//store into global map;
	callm->collect_call_id_summary_unit(ci, summary_unit);

	callm->collect_ci_mp(ci, this);

}


void MtdInstsProcessor::set_call_manager(CallManager* callma) {
	callm = callma;
}


void MtdInstsProcessor::add_to_two_sets(
		sail::Instruction* ins) {

	string str_inst = ins->to_string();

	if (0) {
		cout << " check if this instruction could be add to set_including "
				<< endl;
		cout << "  ^^^^^^^^^^^^^^^^ ^^^^^^^^^^^^ " << rule_to_string() << endl
				<< endl;
	}

	if(track_unique_inst.count(str_inst) > 0)
		return;



	set_include.insert(ins);
	track_unique_inst.insert(str_inst);

}

#ifdef TERMINAL_SIDES_OPEN
/************
 * fill 3 members : set<SideNode*> sides;
 *  set<SideNode*> terminal_sides;
 *  map<sail::Instruction*, set<SideNode*> > inst_rhsnode;
 */
void MtdInstsProcessor::update_side_nodes(sail::Instruction* inst){

	//first build ins;
	one_side* lhs = NULL;
	get_lhs_one_side(inst, lhs);

	set<one_side*>* rhs = NULL;
	get_rhs_one_side(inst, rhs);

//	cout << "		()()()()check before updating " << endl;
//	SideNode::print_sidenodes();
//	cout << "		-------------- ----------- - - " << endl << endl;
//	this->print_terminal_sides();
//	cout << endl << endl;

	SideNode* lhs_sn = SideNode::make(lhs);
	for(auto it = rhs->begin(); it != rhs->end(); it ++){
		SideNode* rhs_sn = SideNode::make(*it);

		lhs_sn->add_to_point_to_container(rhs_sn, inst);

		if(terminal_sides.count(lhs_sn) > 0)
			terminal_sides.erase(lhs_sn);

		rhs_sn->add_to_pointed_by_container(lhs_sn);
//		cout << "		** check generated rhs_sn " << rhs_sn->to_string() << endl;
//		cout << "		** rhs_sn has point to ? " << rhs_sn->has_point_to() << endl;
		if(!rhs_sn->has_point_to()){ //never be in lhs;
			if(terminal_sides.count(rhs_sn) == 0)
				terminal_sides.insert(rhs_sn);
		}
	}
//	cout << "		check after updating ()()()()" << endl;
//	SideNode::print_sidenodes();
//	cout << " 		-- - - - " << endl << endl;
//	this->print_terminal_sides();
//
//	cout << "		(())end of check(()) ! " << endl;

}


void MtdInstsProcessor::get_lhs_one_side(sail::Instruction* inst,
		one_side*& lhs_side){
	switch (inst->get_instruction_id()) {
		case sail::CAST: {
			sail::Cast* cast = (sail::Cast*) inst;
			lhs_side = new one_side(cast->get_lhs());
			return ;
		}


		case sail::ASSIGNMENT: {
			sail::Assignment* assi = (sail::Assignment*) inst;
			lhs_side = new one_side(assi->get_lhs());
			return ;
		}


		case sail::STORE: {
			sail::Store* s = (sail::Store*) inst;
			lhs_side  = new one_side(s->get_lhs(), s->get_offset());
			return ;
		}

		case sail::LOAD: {
			sail::Load* l = (sail::Load*) inst;
			lhs_side = new one_side(l->get_lhs());
			return ;
		}


		case sail::BINOP: {
			sail::Binop* binop = (sail::Binop*) inst;
			lhs_side = new one_side(binop->get_lhs());
			return ;
		}


		case sail::UNOP: {
			sail::Unop* unop = (sail::Unop*) inst;
			lhs_side = new one_side(unop->get_lhs());
			return ;
		}


		default: {
			return;
		}



}
}

void MtdInstsProcessor::get_rhs_one_side(
		sail::Instruction* inst, set<one_side*>*& rhs_sides){

	rhs_sides = new set<one_side*>();

	assert(rhs_sides != NULL);

	switch (inst->get_instruction_id()) {
	case sail::CAST: {
		sail::Cast* cast = (sail::Cast*) inst;
		one_side* rhs = new one_side(cast->get_rhs());
		rhs_sides->insert(rhs);
		return ;
	}



	case sail::ASSIGNMENT: {

		sail::Assignment* assi = (sail::Assignment*) inst;
		one_side* rhs = new one_side(assi->get_rhs());
		rhs_sides->insert(rhs);
		return ;
	}


	case sail::STORE: {
		sail::Store* s = (sail::Store*) inst;
		one_side* rhs = new one_side(s->get_rhs());
		rhs_sides->insert(rhs);
		return ;
	}


	case sail::LOAD: {

		sail::Load* l = (sail::Load*) inst;
		one_side* rhs = new one_side(l->get_rhs(), l->get_offset());
		rhs_sides->insert(rhs);
		return ;
	}


	case sail::BINOP: {

		sail::Binop* binop = (sail::Binop*) inst;

		if(binop->get_first_operand()->is_variable()){
			one_side* rhs1 = new one_side(binop->get_first_operand());
			rhs_sides->insert(rhs1);
		}

		if(binop->get_second_operand()->is_variable()){
			one_side* rhs2 = new one_side(binop->get_second_operand());
			rhs_sides->insert(rhs2);
		}
		return ;
	}


	case sail::UNOP: {

		sail::Unop* unop = (sail::Unop*) inst;

		if (unop->get_operand()->is_variable()) {
			one_side* rhs = new one_side(unop->get_operand());

			rhs_sides->insert(rhs);
		}

		return ;
	}


	default: {
		return;
	}

	}
}
#endif

void MtdInstsProcessor::add_to_in_rules(sail::Instruction* inst){

	//cout << endl <<"		%%%%%%%%%% add_to_in_rules function " << endl;
	int f = add_to_set_including(inst);
	//cout << " 		if add_to_set_including sucessful? " << f << endl;
	//possible to collect lhs and rhs for the instruction;

#ifdef TERMINAL_SIDES_OPEN
	if(f > 0)
		update_side_nodes(inst);
#endif
}

int MtdInstsProcessor::add_to_set_including(sail::Instruction* ins) {

	switch (ins->get_instruction_id()) {

	case sail::STORE:
	case sail::LOAD:
	case sail::BINOP:
	case sail::CAST:{
		add_to_two_sets(ins);
		return 1;
	}

	case sail::UNOP:
	case sail::ASSIGNMENT:{
		if(((sail::Assignment*)ins)->get_rhs()->is_constant())
			return -1;
		add_to_two_sets(ins);
		return 1;
	}

	default:{
		return -1;
	}

//
//	case sail::ASSIGNMENT: {
//
//		sail::Assignment* assi = (sail::Assignment*) ins;
//		add_to_two_sets(assi);
//		return 1;
//	}
//
//
//	case sail::STORE: {
//		sail::Store* s = (sail::Store*) ins;
//		add_to_two_sets(s);
//		return 1;
//	}
//
//
//	case sail::LOAD: {
//		sail::Load* l = (sail::Load*) ins;
//		add_to_two_sets(l);
//		return 1;
//	}
//
//
//	case sail::BINOP: {
//
//		sail::Binop* binop = (sail::Binop*) ins;
//		add_to_two_sets(binop);
//		return 1;
//	}
//
//
//	case sail::UNOP: {
//
//		sail::Unop* unop = (sail::Unop*) ins;
//		add_to_two_sets(unop);
//		return 1;
//	}
//
//
//	default: {
//		//used to catch other type of instructions
//		cerr << "INS:: " << ins->to_string()
//				<< "   don't need to be added as fix point instructions ! " << endl<<endl;
//		return -1;
//	}


	}

}


void MtdInstsProcessor::store_into_unresolved_fc_map(
		sail::FunctionCall* fc) {

	assert(fc != NULL);
	//this phase find all possible callees that fit this function call signature;
	//1)directly function signature;
	string callee_name = fc->get_function_name();
	il::type* sig = fc->get_signature();
	call_id ci(callee_name, fc->get_namespace(), sig, lang_type::LANG_JAVA);

	set<call_id> targets;
	targets.insert(ci);



	//2)if it's duplicated fun-sig, then adding the dummy ones;
	if(callm->is_dummy_callid(ci)){
		set<call_id>& dummy_callids = callm->get_dummy_duplicated_ids(ci);
		targets.insert(dummy_callids.begin(), dummy_callids.end());

		if(0){
			cout << "SSize of dummy call_id is " << dummy_callids.size() << endl;
			cout << "Inserting dummy callid " <<
					(*dummy_callids.begin()).to_string()
					<< "	 for " << fc->to_string() << endl;
		}
	}

	//3) virtual function calls;
	find_virtual_targets(ci, fc, targets);

	if(0){
		cout << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHello begin!! " << endl;
		for(auto it = targets.begin(); it != targets.end(); it++){
			cout << "target is " << (*it).to_string() << endl;
		}
		cout << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHello end!! " << endl;
	}



	for(set<call_id>::iterator it = targets.begin();
			it != targets.end();
			it++) {
		call_id cur_ci = *it;

		if(callm->is_irreducible_method(cur_ci))
			continue;

		//for the purpose of skip the library functions;
		if (!callm->inside_collect_call_ids(cur_ci))
			continue;

		//callee's id, and fc that used to update this method;
		if (unresolved_fc.count(cur_ci) > 0) {
			unresolved_fc[cur_ci]->push_back(fc);
		} else {
			vector<sail::FunctionCall*>* fcs =
					new vector<sail::FunctionCall*>();
			fcs->push_back(fc);
			unresolved_fc[cur_ci] = fcs;
		}
	}

}




void MtdInstsProcessor::process_first_round_single_instruction(sail::Function* f,
		sail::Instruction* ins, unsigned int i,
		const string& output_folder) {

	cout << endl << "************************ " << endl;
	cout << "Processing instruction  " << "   [ " << i << "]" << endl
			<< ins->to_string() << endl;



	if (ANALYZE_INS) {
		if (PROCESS_STAITC_ASSERT)
			analysis_ins(i, ins);
		else
			analysis_ins_without_static_assert(i, ins);

		cout << endl << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ END!" << endl;

		//output check
//		string fun_name = f->get_namespace().to_string()
//				+ f->get_identifier().get_function_name() + "inst"
//				+ int_to_string(i);

		if (WRITE_TO_DOT_FILES)
			write_to_dotfiles(f->get_identifier(),
					"inst" + int_to_string(i), output_folder, mem_graph);
	}

}


void MtdInstsProcessor::simplify_instructions_in_rule(
		set<sail::Instruction*>& to_delete)
{
	//auto iter =
}



void MtdInstsProcessor::process_instructions_in_rule() {

	cout << endl << endl<< " 2)= == = == = = process_instructionsd_in_rule() = ==   = = = = =  " << endl;
	cout << get_time() << endl;

#ifdef TERMINAL_SIDES_OPEN
	this->print_terminal_sides();
#endif

	//exit(1);


	bool fix_point = false;

	int i = 0;


	//	set<sail::Instruction*> to_delete;
	//	simplify_instruction_in_rule(to_delete);


	set<MemEdge*>* edges = mem_graph->get_edges();

	while (!fix_point) {

		//set<MemEdge*>* before_edges = mem_graph->get_edges();

		int before_size = edges->size();

		cout << " == Round " << i++ << endl;
		cout << "set_include size " << set_include.size() << endl;

		int j = 0;
		for (set<sail::Instruction*>::iterator it = set_include.begin();
				it != set_include.end(); it++) {
			//cout << "---- == ---- index for instruction in rules " << j++ << endl;
			//cout << " ==** Time before " << get_time() << endl;

			sail::Instruction* ins = *it;

			int pre_size = edges->size();

			InsMemAnalysis pit(ins, this);

			pit.inprecise_ins_analysis(FIND_FIX_POINT);

			int post_size = edges->size();

			if (1) {
				if (pre_size < post_size)
					cout << ins->to_string() << " add NEW EDGES "
					<< " shouldn't be fix point " << endl;
			}

			//			write_to_dotfiles(f->get_identifier().to_string() + "rule_test" + int_to_string(i), "/scratch/mem_graph/",
			//					get_mem_graph());
			//cout << " ==** Time after ? " <<get_time() << endl;
			//cout << "------------------------------ " << endl;

		}


		int after_size = edges->size();

		if (before_size == after_size)
			fix_point = true;

		//i++;

	}

	write_to_dotfiles(f->get_identifier(), "rule_test", "/scratch/mem_graph/",
			get_mem_graph());
	cout << " ==** Time after ? " <<get_time() << endl;
	cout << "------------------------------ " << endl;
	cout << "= = = = = = = end of process_instructionsd_in_rule() " << endl;
}





void MtdInstsProcessor::collect_static_assert_boundaries() {
	for (unsigned int i = 0; i < f->get_body()->size(); i++) {

		sail::Instruction* cur_inst = f->get_body()->at(i);
		assert(cur_inst != NULL);

		a_builder->set_instruction(cur_inst);

		a_builder->collect_static_assert_boundary();
	}
}

void MtdInstsProcessor::building_static_assert_eles() {
	if (a_builder->has_static_assert()){
		vector<sail::Instruction*>* body = f->get_body();
		a_builder->buildup_static_assert_element(body);
	}

}

void MtdInstsProcessor::set_up_complete_static_assert_eles() {

	collect_static_assert_boundaries();

	if (0)
		a_builder->test_static_assert_boundary();

	building_static_assert_eles();
}

const string MtdInstsProcessor::unresolved_functioncall_to_string() const{

	string str;

	for(map<call_id, vector<sail::FunctionCall*>*>::const_iterator
			it = unresolved_fc.begin();
			it != unresolved_fc.end(); it ++){
		str += "\n";
		str += "########## ";
		call_id ci = it->first;

		str +=  ci.to_string();
		str += "\n";

		int i = 0;
		vector<sail::FunctionCall*>* insts = it->second;
		for(vector<sail::FunctionCall*>::iterator it1 = insts->begin();
				it1 != insts->end(); it1 ++){
			i++;
			str += "	[ ";
			str += int_to_string(i);
			str += " ]	";
			sail::FunctionCall* ins = *it1;
			str += "\n";
			str += "		== ";
			str += ins->to_string();
			str += "	linenum: ";
			str += int_to_string(ins->get_line_number());

		}
		str += "\n";
	}

	return str;

}


const void  MtdInstsProcessor::unresolved_functioncall_test()const{


	for(map<call_id, vector<sail::FunctionCall*>*>::const_iterator it
			= unresolved_fc.begin();
			it != unresolved_fc.end(); it ++){

		call_id ci = it->first;
		cout << "CallId :: " << ci.to_string() << endl;

		vector<sail::FunctionCall*>* insts = it->second;

		for(vector<sail::FunctionCall*>::iterator it1 = insts->begin();
				it1 != insts->end(); it1 ++){

			sail::FunctionCall* ins = *it1;
			cout << "INS :: " << ins->to_string() << endl;
		}
	}
}



void MtdInstsProcessor::apply_summary_unit_to_func(sail::FunctionCall* fc,
		SummaryUnit* callee_su, bool& updated){

	list<sail::Instruction*> insts = callee_su->get_equivalent_insts();

	map<string, sail::Variable*> summary_temp_var_map;
	cerr << endl << endl << "********** call-site      " << fc->to_string() <<
			"   "  << " need to process << " << insts.size() << " summarized instructions!!!"
			<< "	@@@ caller's file line is " << fc->get_line_number() << endl;
	cerr << "Total instruction inside summarization unit is " << insts.size() << endl;
	int i = 0;


	for(list<sail::Instruction*>::iterator it = insts.begin();
			it != insts.end(); it++) { //instruction inside callee's summarization;
		cout << "*****************************" << i <<  "********************************  " << endl;
		sail::Instruction* gen = NULL;
		replace_symbols_with_arguments(*it, fc, gen, summary_temp_var_map);

		cout << i++ << "     [[[[[  changes to => " << gen->to_string() << endl;

		assert(gen != NULL);

		//before processing, it is necessary to check if in_use variables are existing ?
		//complete it later for other in_use variables

		if (0) {
			if (gen->get_instruction_id() == sail::STORE) {
				sail::Symbol* sym = ((sail::Store*) gen)->get_lhs();
				assert(sym->is_variable());

				AccessPath* ap = get_ap_from_symbol(sym, this);
				if (!get_mem_graph()->has_location(ap))
					updated = true;

			}

			if (updated)
				return;
		}

		//add_to_set_including(gen);

		this->add_to_in_rules(gen);

		cout << endl << endl << endl<< endl << endl<<" "
				"&&&&&&&&&& processing instruction in summary unit " << endl;
		InsMemAnalysis  pit(gen, this);

		pit.inprecise_ins_analysis(BASIC);

		if(0){
		//if (WRITE_TO_DOT_FILES) {
			cout << "Write to file for == > " << this->get_function()->get_identifier().to_string() <<
					"_apply_sum_inst" <<  int_to_string(i) << endl;
			cout << "???????????????? really ??????????????????? " << endl;
			write_to_dotfiles(this->get_function()->get_identifier(),
					"_apply_sum_inst" + int_to_string(i), output_folder,
					this->get_mem_graph());
			cout << endl << endl;
		}
		cout << "analysis end of instruction " << i << "	in summary unit!" <<endl;
		if(gen->get_instruction_id() == sail::FUNCTION_CALL){
			sail::FunctionCall* fc = (sail::FunctionCall*)gen;
			assert(fc->is_allocator());
		}
		cout << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHH " <<  i << endl;
		//assert(gen->get_instruction_id() != sail::FUNCTION_CALL);


		if (0) {

			if (gen->get_instruction_id() != sail::FUNCTION_CALL)
				continue;

			sail::FunctionCall* cur_fc = (sail::FunctionCall*) gen;

			if (cur_fc->is_allocator())
				continue;

			assert(cur_fc->is_constructor());

			//find its corresponding callee which should be resolved;

			//put it to the map;
			call_id ci(cur_fc->get_function_name(), cur_fc->get_namespace(),
					cur_fc->get_signature(), lang_type::LANG_JAVA);

			assert(callm->is_resolved(ci));
			//SummaryUnit* find_callee_su = callm->get_resolved_summary_unit(ci);
			//second_map[cur_fc] = find_callee_su;
			//apply_summary_unit_to_func(cur_fc, find_callee_su);
		}


	}
	cout << "************************************ outside loop ******************* " << endl;
}



const string MtdInstsProcessor::rule_to_string() const {

	string str;

	if (set_include.size() == 0)
		return "EMPTY!!!";

	str += "SIZE : " ;
	str += int_to_string(set_include.size());
	set<sail::Instruction*>::const_iterator it = set_include.begin();

	for (; it != set_include.end(); it++) {

		str += " === > ";

		str += (*it)->to_string();
		str += "\n";

	}

	return str;

}

void MtdInstsProcessor::update_unresolved_fc_from_callid(const call_id& ci){
		if(unresolved_fc.count(ci) <= 0)
					cerr << "ci is " << ci.to_string() << endl;
		if(WRITE_TO_TEST_FILE){
			MtdInstsProcessor::callm->ofs << "DDDDDDDDDDDDeleting " << ci.to_string() <<
					"    from unresolved_fc!!!" << endl;
		}

		cerr << "DDDDDDDDDDDDeleting " << ci.to_string() <<
				"    from unresolved_fc!!!" << endl;

		assert(unresolved_fc.count(ci) > 0);
		unresolved_fc.erase(ci);
}

void MtdInstsProcessor::fill_alloc_context_controller(AccessPath* lhs_ap,
		AccessPath* alloc_ap, sail::FunctionCall* alloc_fc){

		assert(lhs_ap != NULL);
		assert(alloc_ap != NULL);
		assert(alloc_fc != NULL);

		alloc_fc->get_lhs()->set_is_point_to_alloc();
		//cout << "Allocation found and build alloc_context_controller " << endl;

		Deref* deref_alloc = Deref::make(alloc_ap);
		MemNode* alloc_deref_node = mem_graph->get_memory_node(deref_alloc);
		//cout << "get alloc_deref_node  " << alloc_deref_node->to_string() << endl;


		AllocContext* allctx = new AllocContext();
		allctx->set_alloc_memnode(alloc_deref_node);
		allctx->set_alloc_ins(alloc_fc);
		allctx->set_alloc_lhs_ap(lhs_ap);

		if(allocc == NULL){
			allocc = new AllocContextController();
			has_alloc = true;
		}

		allocc->collect_alloc_context(allctx, deref_alloc);
}




const void MtdInstsProcessor::print_alloc_context_insts() const{
	cout << allocc->to_string() << endl;
}


void MtdInstsProcessor::analysis_ins_without_static_assert(
		unsigned int i,
		sail::Instruction* ins) {

	if(ins->is_reachable()){
		InsMemAnalysis pit(ins, this, i);
		pit.inprecise_ins_analysis(BASIC);
	}
}



void MtdInstsProcessor::analysis_ins(unsigned int i, sail::Instruction* ins) {
	if(!ins->is_reachable())
		return;

	//if ins  contains invalid type, it shouldn't true
	if(inst_contain_invalid_type(ins)){
		assert(false);
	}


	if (!(ins->is_inside_static_assert())) {
		cout << "|| >> processing normal ins analysis << || "
				<< ins->to_string() << endl;
		if (ins->is_reachable()) {
			InsMemAnalysis pit(ins, this, i);
			pit.inprecise_ins_analysis(CHECK_BEFORE_BBS);
		}

	} else { //other auxiliary instructions should be ignored(because it need to be precisely processed)

		if (ins->get_instruction_id() == sail::FUNCTION_CALL) {

			sail::FunctionCall* fun_c = static_cast<sail::FunctionCall*>(ins);
			if (fun_c->get_function_name().find(STATIC_ASSERT_BEGIN)
					!= string::npos) {

				cout << "|| >> processing static_assert << || " << endl;

				StaticAssertElement* ele =
						a_builder->get_global_assert()->get_static_assert_ele(
								fun_c);

				a_builder->process_static_assert_element(ele);
			}
		}
	}
}



/*
 void deleteIns(sail::Instruction*  ins, set<sail::Instruction*>* labels){
 if((ins != NULL)&&(!(ins->get_instruction_id() == sail::instruction_type::SAIL_LABEL)) ){//one label may be refered by other instructions multiple times
 //cout << "deleting ------------- " << ins->to_string() << endl << endl << endl;
 delete ins;
 ins = NULL;
 }else{
 if(labels == NULL)
 labels = new set<sail::Instruction*> ();
 labels->insert(ins);
 //cout << "not deleting ---------------- " << ins->to_string() << endl << endl << endl;
 }
 }


 void deleteLabels(set <sail::Instruction*>* const labels){
 if (labels != NULL) {
 set<sail::Instruction*>::iterator it_label = labels->begin();
 for (; it_label != labels->end(); it_label++) {
 *it_label;
 }
 delete labels;
 }
 }*/
