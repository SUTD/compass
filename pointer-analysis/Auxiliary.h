#ifndef _AUXILIARY_H
#define _AUXILIARY_H

#include "sail.h"
#include <set>
#include "StaticAssertElement.h"
#include "il.h"
#include "TwoBranchConstraints.h"
#include "Constraints.h"
#include "MemGraph.h"
#include "InsMemAnalysis.h"
#include "access-path.h"
#include "Term.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>

//#include "SummaryUnit.h"


using namespace std;
#define PROCESS_STAITC_ASSERT false
class target;



void indent_print(int level);

string operator_to_string(long int op);

bool is_terminal_term(Term* t) ;
bool is_terminal_term(AccessPath* ap);

bool reflexible_operator(long int op);
vector<AccessPath*> get_sub_aps(AccessPath* ap);
il::binop_type get_binop_type(long int);

sail::Variable* get_sail_variable(AccessPath* ap);

const call_id& identifier_to_call_id(const Identifier& id);

string get_argument_name(int arg_num);

string get_temp_name();



void replace_symbols_with_arguments(sail::Instruction* replaced_ins,
		sail::FunctionCall* fc, sail::Instruction*& generated_ins,
		map<string, sail::Variable*>& var_map);

void produce_new_line_number(const string& allo_site_str, int& new_line);



void copy_instruction(sail::Instruction* base,
		sail::Instruction*& generated_ins);

sail::Symbol* clone(sail::Symbol* base);

AccessPath* get_ap_from_symbol(sail::Symbol* s, MtdInstsProcessor* mtdp);

void find_virtual_targets(const call_id& ci,
		sail::FunctionCall* fc, set<call_id>& targets);

void find_virtual_dispatch(call_id& ci, sail::FunctionCall* fc);





//for the purpose walk around the instruction who has invalid type
bool inst_contain_invalid_type(sail::Instruction* ins);






/****
 * for the purpose of finding tracked call_id has been tracked again,
 * which generated a circle;
 *
 * return the idx inside the vector that find the given call_id;
 *
 * if return is -1, didn't find the given call_id inside the collect vector
 */

int find_in_tracked_call_ids(vector<call_id>& call_ids, const call_id& call_id);









/*******
 * first_f means that it is the entry, no constraintItem provided, if(first_f = true, con_i = NULL)
 * otherwise con_i should provided
 */
void binop_operands_both_constants(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& op1_t, target& op2_t, il::binop_type bt,
		TwoBranchConstraints*& two_br_con, bool first_f,
		ConstraintItem* con_i);

void binop_operd1_const_operd2_ap(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& op1_t, target& op2_t, il::binop_type bt,
		TwoBranchConstraints*& two_br_con, bool first_f,
		ConstraintItem* con_i);

void binop_operd1_ap_operd2_const(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& op1_t, target& op2_t, il::binop_type bt,
		TwoBranchConstraints*& two_br_con, bool first_f,
		ConstraintItem* con_i);

void binop_operands_both_aps(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& op1_t, target& op2_t, il::binop_type bt,
		TwoBranchConstraints*& two_br_con, bool first_f,
		ConstraintItem* con_i) ;

void adding_two_new_constraints(sail::Symbol* oper1_s,
		sail::Symbol* oper2_s,/* set<long int>& oper1_set, set<long int>& oper2_set,*/
		target& op1_t, target& op2_t,
			il::binop_type bt, TwoBranchConstraints*&,
			bool first_f, ConstraintItem* con_i);

////////////////////////////////////////////////////////////////
void both_consts_one_exist(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& oper1_t, target& oper2_t,
		il::binop_type bt, ConstraintItem* con_item,
		TwoBranchConstraints*& two_br_con, bool oper_exist);

void oper1_ap_oper2_const_one_exist(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& oper1_t, target& oper2_t,
		il::binop_type bt, ConstraintItem* con_item,
		TwoBranchConstraints*& two_br_con, bool oper1_exist);

void operd1_const_operd2_ap_one_exist(sail::Symbol* oper1_s,
		sail::Symbol* oper2_s,
		target& oper1_t,
		target& oper2_t,
		il::binop_type bt,
		ConstraintItem* con_item,
		TwoBranchConstraints*& two_br_con,
		bool oper1_exist);

void both_aps_one_exist(sail::Symbol* oper1_s,
		sail::Symbol* oper2_s,
		target& oper1_t,
		target& oper2_t,
		il::binop_type bt,
		ConstraintItem* con_item,
		TwoBranchConstraints*& two_br_con,
		bool oper1_exist);

void update_existing_constraints(Constraints* con,
		sail::Symbol* oper1_s,
		sail::Symbol* oper2_s, /*set<long int>& oper1_value_set,
		set<long int>& oper2_value_set,*/
		target& oper1_t,
		target& oper2_t,
		il::binop_type bt,
		TwoBranchConstraints*&);

bool is_bt_cmp_true(il::binop_type bt,
		long int oper1_value,
		long int oper2_value);

bool binop_type_need_to_process(il::binop_type bt);

void get_connect_blockids(sail::Block* begin,
		sail::Block* end,
		vector<int>& block_ids);


/***********
 * write memory graph to dot file;
 * para "id", function id;
 * para "append_str" used to disguish different phrases to
 * draw mem-graph, as you want to add;
 * para "out_put_folder " the folder that you provide to put the file
 * para "mg"  the target to draw;
 */
void write_to_dotfiles(const Identifier& id,
		const string& append_str,
			const string& out_put_folder,
			MemGraph* mg);

int get_index_of_label(vector<sail::Instruction*>*& body,
				sail::Label* l);

bool comparable_bt(il::binop_type bt);
void print_inst(sail::Instruction* ins);
bool is_bt_cmp_ap_true(il::binop_type bt,
		AccessPath* ap1,
		AccessPath* ap2);
bool is_eq_bt(il::binop_type bt);



void collect_arith_fun_aps(AccessPath* cur_ap,
		vector<AccessPath*>& aps,
		bool& arg_flag);


bool access_paths_has_argument(vector<AccessPath*>& aps);
bool access_paths_has_alloc(vector<AccessPath*>& aps);
void collect_access_paths(Term* t, vector<AccessPath*>& aps);


sail::Symbol* get_variable(Term* t);
void get_unop_code(long int value, il::unop_type& unop);
void get_binop_code(long int value, il::binop_type& binop);


//for the purpose of create legal .dot files when draw memory graph
void _mkdir(const char *dir);

string replace_illegal_characters(const string& s);

string get_legal_filename(string  directory,
		const string & _identifier);

void open_file(ofstream& ofs, const string& file);



void print_ap_type(AccessPath*);

bool compatible_for_function_call(il::type* argument,
		il::type* parameter);

#endif
