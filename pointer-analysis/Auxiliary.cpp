#include "Auxiliary.h"

#define MAX_FILENAME_SIZE 110

/*
 * The minimum size of any "chunk" created by splitting a file
 * into folder/.../file
 */
#define MIN_FILNAME_SIZE 10
#define JAVA_OBJECT_POINTER "Ljava/lang/Object;*"
#define JAVA_OBJECT "Ljava/lang/Object;"

#define CONSIDER_ARITH_OF_ALLOC true

int temp_idx = 0;
int counting = 0;

map<string, set<int> > allocsites_line_map;
set<int> alloc_line_number;

void indent_print(int level){
    for (int i = 1; i< level; i++)
	cout << "\t";
}

string operator_to_string(long int op) {
	string str;
	switch (op) {
	case -2:
		str = "-";
		break;
	case -1:
		str = "+";
		break;
	case 0: //bitwise_not
		str = "~";
		break;
	case 1:
		str = "&";
		break;
	case 2:
		str = "|";
		break;
	case 3:
		str = "^";
		break;
	case 4:
		str = "<<";
		break;
	case 5:
		str = ">>";
		break;
	case 6:
		str = "%";
		break;
	case 13:
		str = "*";
		break;
	case 14:
		str = "/";
		break;
	default:
		str = " ";
		break;
	}
	return str;
}


bool is_terminal_term(Term* t) {
	switch (t->get_term_type()) {
	case CONSTANT_TERM:
	case VARIABLE_TERM:
		return true;
	case FUNCTION_TERM:{
		if(AccessPath::to_ap(t)->get_ap_type() == AP_FIELD)
			return true;

		return false;
	}
	case ARITHMETIC_TERM:{

	}

	default:
		return false;
	}
}

bool is_terminal_term(AccessPath* ap){
	return is_terminal_term(ap->to_term());
}

bool reflexible_operator(long int op){
	if(op == 1)
		return true;
	if(op == 13)
		return true;
	return false;
}


il::binop_type get_binop_type(long int value){
	il::binop_type bt;
	switch (value) {
	case -2:
		bt = il::_MINUS;
		break;
	case -1:
		bt = il::_PLUS;
		break;

//	case 0:
//		bt = il::_BITWISE_NOT;
//		break;
	case 1:
		bt = il::_BITWISE_AND;
		break;
	case 2:
		bt = il::_BITWISE_OR;
		break;
	case 3:
		bt = il::_BITWISE_XOR;
		break;
	case 4:
		bt = il::_LEFT_SHIFT;
		break;
	case 5:
		bt = il::_RIGHT_SHIFT;
		break;
	case 6:
		bt = il::_MOD;
		break;
	case 13:
		bt = il::_MULTIPLY;
		break;
	case 14:
		bt = il::_DIV;
		break;
	default: {
		cout << "binop is wrong!!  " << value << endl;
		assert(false);
	}
		break;
	}
	return bt;
}



sail::Variable* get_sail_variable(AccessPath* ap){
	//cout << "$$$$$$$$$$$$ var " << ap->to_string() << " and ap type " << ap->get_ap_type()<< endl;
	if(ap->get_ap_type() == AP_VARIABLE)
		return ((Variable*)ap)->get_original_var();

	if(ap->get_ap_type() == AP_FIELD){

		AccessPath* inner_ap = ap->get_inner();
		AccessPath* inner_inner = NULL;
		if(inner_ap->get_ap_type() == AP_DEREF)
			inner_inner = ((Deref*)inner_ap)->get_inner();
		if (inner_inner != NULL) {
			if (inner_inner->get_ap_type() == AP_VARIABLE)
				return ((Variable*) inner_inner)->get_original_var();
		}
	}
	return NULL;
}



const call_id& identifier_to_call_id(const Identifier& id) {

	call_id* ci = new call_id(id.get_function_name(), id.get_namespace(),
			id.get_function_signature(), id.get_language());

	return *ci;
}




string get_argument_name(int arg_num) {
	string arg_name = "p" + int_to_string(arg_num);
	return arg_name;
}


string get_temp_name(){
	string temp_name = "_temp_" + int_to_string(counting++);
	return temp_name;
}

sail::Symbol* clone(sail::Symbol* base){

	if(base == NULL)
		return NULL;

	if(base->is_constant())
		return ((sail::Constant*)base)->clone();
	else
		return ((sail::Variable*)base)->clone();

}



AccessPath* get_ap_from_symbol(sail::Symbol* s,
		MtdInstsProcessor* mtdp){

	assert(s != NULL);
	if(s->is_constant()){
		sail::Constant* c = (sail::Constant*) s;
		if(c->is_integer())
			return ConstantValue::make(c->get_integer());
		else {
			DisplayTag dt;
			return UnmodeledValue::make_imprecise(s->get_type(), dt);
		}
	}

	sail::Variable* v = (sail::Variable*)s;
	if(0)
		cout << "=====>Processing var " <<v->to_string()
			<<endl <<  "Address :: " << v << "     " <<
			" inside  "<< mtdp->get_function()->get_identifier().to_string() <<endl;
	if(mtdp->find_symbol(v)){ //directly unique them;
		if(0)
			cout << "  =>find  " << v->to_string() << endl;
		Variable* var = (Variable*)mtdp->get_ap(v);
		return var;
	}else { //make sure do not use the same ap from other method but with same name but others different;

		Variable * var = Variable::make_program_var(v);
		sail::Variable* origi_v = v;
		while(!(*v == *(var->get_original_var()))){

			v->set_name(v->get_var_name()+ "_" +int_to_string(temp_idx++));
			Variable* new_var = Variable::make_program_var(v);
			var = new_var;
		}
		mtdp->store_to_symbols(v, var);
		if(0)
			cout << "  =>store  " << origi_v->to_string() << " As " << var->to_string() << endl;
		return var;
	}
}



void find_virtual_targets(const call_id& ci,
		sail::FunctionCall* fc, set<call_id>& targets)
{

	if(!fc->is_virtual_call())
		return;

	assert(fc->get_arguments()->size() != 0);

	fc->get_virtual_call_targets(targets);


	//only for test use
	if(0){
		if(targets.size() > 1){
			for(set<call_id>::iterator it = targets.begin();
					it != targets.end(); it++){
				cout << "** || virtual target is " << (*it).to_string() << endl;
			}
		}
	}
}



void find_virtual_dispatch(call_id& ci, sail::FunctionCall* fc)
{

	if(!fc->is_virtual_call())
		return;

	assert(fc->get_arguments()->size() != 0);

	sail::Symbol* s = fc->get_arguments()->at(0);

	assert(s->is_variable());

	il::type* arg_t = ((sail::Variable*) s)->get_type();

	//itself and all its sub_class
	cerr << "declared call_id " << ci.to_string() << endl;
	set<call_id> targets;
	targets.insert(ci);

	fc->get_virtual_call_targets(targets);

	cerr << "targets size ==>" << targets.size() << endl;
	for (set<call_id>::iterator it = targets.begin();
			it != targets.end(); it++) {
		cerr << " %%%%% call id :::" << (*it).to_string() << endl;
		il::function_type* ft =
				(il::function_type*) ((*it).get_signature());
		assert(ft->get_arg_types().size() != 0);

		il::type* t = ft->get_arg_types().at(0);

		if (arg_t == t) {
			ci = *it;
			cerr << "find fitting call id is " << ci.to_string()
					<< endl;
		}
	}
}



bool inst_contain_invalid_type(sail::Instruction* ins)
{
	switch (ins->get_instruction_id()) {

	case sail::ASSIGNMENT: {
		sail::Assignment* assi = (sail::Assignment*) ins;
		if (assi->get_lhs()->get_type()->is_invalid_type())
			return true;
		if (assi->get_rhs()->get_type()->is_invalid_type())
			return true;

		return false;
	}
		break;

	case sail::STORE: {
		sail::Store* s = (sail::Store*) ins;

		if (s->get_lhs()->get_type()->is_invalid_type())
			return true;

		if (s->get_rhs()->get_type()->is_invalid_type())
			return true;

		return false;
	}
		break;

	case sail::LOAD: {
		sail::Load* l = (sail::Load*) ins;

		if (l->get_lhs()->get_type()->is_invalid_type())
			return true;

		if (l->get_rhs()->get_type()->is_invalid_type())
			return true;

		return false;
	}
		break;

	case sail::BINOP: {
		sail::Binop* b = (sail::Binop*) ins;

		if (b->get_lhs()->get_type()->is_invalid_type())
			return true;

		if (b->get_first_operand()->get_type()->is_invalid_type())
			return true;

		if (b->get_second_operand()->get_type()->is_invalid_type())
			return true;

		return false;
	}
		break;

	case sail::UNOP: {
		sail::Unop* r = (sail::Unop*) ins;

		if (r->get_lhs()->get_type()->is_invalid_type())
			return true;

		if (r->get_operand()->get_type()->is_invalid_type())
			return true;

		return false;
	}
		break;

	case sail::CAST: {
		sail::Cast* c = (sail::Cast*) ins;

		if (c->get_lhs()->get_type()->is_invalid_type())
			return true;

		if (c->get_rhs()->get_type()->is_invalid_type())
			return true;

		return false;
	}
		break;


	case sail::FUNCTION_CALL:{
		sail::FunctionCall* fc = (sail::FunctionCall*)ins;

		assert(fc->is_allocator());

		if(fc->get_lhs()->get_type()->is_invalid_type())
			return true;

		return false;

	}break;

	case sail::ADDRESS_STRING:{

		sail::AddressString* addr_str =  (sail::AddressString*) ins;

		if(addr_str->get_lhs()->get_type()->is_invalid_type())
			return true;

		return false;

	}break;

	default: {
		cerr << "Find inst " << ins->to_string() << endl;
		cerr << "instruction id is " << ins->get_instruction_id() << endl;
		assert(false);
	}
	}


}

void copy_instruction(sail::Instruction* base,
		sail::Instruction*& generated_ins)
{

	//cerr << "inside copy_instruction " << endl;
	switch(base->get_instruction_id()){
	case sail::ASSIGNMENT:{

		sail::Assignment* r = (sail::Assignment*)base;

		sail::Assignment* assi = new sail::Assignment(
				(sail::Variable*)(clone(r->get_lhs())),
				clone(r->get_rhs()), r->get_line_number());



		generated_ins = assi;
	}
	break;
	case sail::STORE:{

		sail::Store* r = (sail::Store*)base;

		sail::Store* s = new sail::Store(clone(r->get_lhs()), clone(r->get_rhs()),
				r->get_offset(), r->get_field_name(), r->get_line_number());

		generated_ins = s;

	}break;
	case sail::LOAD:{

		sail::Load* r = (sail::Load*)base;
		sail::Load* l = new sail::Load((sail::Variable*)clone(r->get_lhs()),
				clone(r->get_rhs()), r->get_offset(), r->get_field_name(),
				r->get_line_number());


		generated_ins = l;

	}break;
	case sail::BINOP:{
		sail::Binop* r = (sail::Binop*)base;

		//first test clone constant to see if it works

		sail::Symbol* s = clone(r->get_second_operand());
		assert(s != NULL);
		cout << "s is " << s->to_string() << endl;

		sail::Binop* bi = new sail::Binop((sail::Variable*)clone(r->get_lhs()),
				clone(r->get_first_operand()),
				clone(r->get_second_operand()),
				r->get_binop(), r->get_line_number());

		generated_ins = bi;

	}break;
	case sail::UNOP:{
		sail::Unop* r = (sail::Unop*)base;
		sail::Unop* u =  new sail::Unop((sail::Variable*)clone(r->get_lhs()),
				clone(r->get_operand()), r->get_unop(), r->get_line_number());

		generated_ins = u;

	}break;

	case sail::FUNCTION_CALL:{

		sail::FunctionCall* fc = (sail::FunctionCall*)base;
		if((fc->is_allocator()) ||(fc->is_constructor())){

			vector<sail::Symbol*>* args = fc->get_arguments();
			vector<sail::Symbol*>* copy_args = new vector<sail::Symbol*>();
			for(int i = 0; i < (int)args->size(); i++){
				sail::Symbol* s = args->at(i);
				sail::Symbol* copy_s = clone(s);
				copy_args->push_back(copy_s);
			}



			sail::FunctionCall* fcall = new sail::FunctionCall(
					(sail::Variable*)clone(fc->get_return_variable()),
					fc->get_function_name(), fc->get_namespace(),
					fc->get_signature(), copy_args,
					fc->is_virtual_call(), fc->is_allocator(), fc->is_constructor(),
					fc->is_super(), fc->get_line_number());

			generated_ins = fcall;
			return;
		}
		assert(false);

	}break;

	default:{
		cerr << "instruction id is " << base->get_instruction_id() << endl;
		assert(false);
	}
	}
}

void produce_new_line_number(const string& alloc_site_str,
		int& line_nu)
{

	assert(alloc_site_str != "");

	if(allocsites_line_map.count(alloc_site_str) == 0)
	{
		set<int> line_set;
		line_set.insert(line_nu);
		allocsites_line_map[alloc_site_str] = line_set;
	}
	else //already existing;
	{
		//get the set;
		set<int>& linemap = allocsites_line_map[alloc_site_str];
		++ line_nu ;
		while(linemap.count(line_nu) > 0){
			++ line_nu;
		}

		linemap.insert(line_nu);
	}

}


//one instruction could only have one unique ret/argument
//base is summary
void replace_symbols_with_arguments(sail::Instruction* base,
		sail::FunctionCall* fc, sail::Instruction*& gen,
		map<string, sail::Variable*>& var_map) {

	bool alloc_f = false;
	string alloc_site_str = "";

	if(base->get_instruction_id() == sail::FUNCTION_CALL){
		sail::FunctionCall* func = (sail::FunctionCall*)base;
		if(func->is_allocator()){
			alloc_f = true;
			alloc_site_str = func->get_function_name();
		}
	}

	if (1) {
		cout << endl <<"Fc is " << fc->to_string() << endl;

		cout << "<<<<<<<<<<< >>>>>>>>>>>>> REF " << base->to_string() << endl;
	}

	copy_instruction(base, gen);

	int line_nu = fc->get_line_number();

	if (alloc_f){
		produce_new_line_number(alloc_site_str, line_nu);
	}


	gen->set_line_number(line_nu);

	if (1)
		cout << "<<<<<<<<<<< >>>>>>>>>>>>> GEN " << gen->to_string() << endl;

	vector<sail::Symbol*> symb_pars;

	sail::Symbol* s = gen->get_lhs();
	symb_pars.push_back(s);
	gen->get_rhs_symbols(symb_pars);

	sail::Symbol* sym = NULL;

	for (int i = 0; i < (int) symb_pars.size(); i++) {

		sym = symb_pars.at(i);

		if (sym == NULL) //return variable(lhs) could be NULL;
			continue;

		assert(sym != NULL);
		if (0)
			cout << " Processing symbol is " << sym->to_string() << endl;
		if (sym->is_variable()) {

			sail::Variable* var = (sail::Variable*) sym;
			//cout << "var " << var->to_string() << endl;
			if (var->is_return_variable()) {
				if(fc->get_lhs() != NULL)
					cout << fc->get_lhs()->to_string() << endl;
				else
					cout << "fc return value is NULL " << endl;
				if(fc->get_lhs() != NULL)
					*var = *(fc->get_lhs());
			} else if (var->is_argument()) {

				vector<sail::Symbol*>* arguments = fc->get_arguments();
				if (0) {
					cout << "fc is " << fc->to_string() << endl;
					cout << "argument size is " << arguments->size() << endl;
					cout << " var : " << var->to_string()
							<< "  at argument size is " << var->get_arg_number()
							<< endl;
				}
				*var =
						*((sail::Variable*) (arguments->at(
								var->get_arg_number())));

			}else { //make a new temp variable
				//if found in the mapping, get it, otherwise build it and put it to map
				if(var_map.count(var->to_string())){
					sail::Variable* s = var_map[var->to_string()];
					*var = *s;
				} else {
					sail::Variable* temp = new sail::Variable(get_temp_name(),
							var->get_type(), true);
					var_map[var->to_string()] = temp;
					*var = *temp;

				}
			}
		}
	}

	if (0)
		cout << "################################ 1" << gen->to_string()
				<< endl;

}








int find_in_tracked_call_ids(vector<call_id>& call_ids,
		const call_id& call_id){

	for(int i = 0; i < (int)call_ids.size(); i ++){
		if(call_id == call_ids.at(i))
			return i;
	}

	return -1;

}










void binop_operands_both_constants(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& op1_t, target& op2_t, il::binop_type bt,
		TwoBranchConstraints*& two_br_con, bool first_f,
		ConstraintItem* con_i) {
	for (set<sail::Constant*>::iterator it1_const =
			op1_t.get_constants().begin();
			it1_const != op1_t.get_constants().end(); it1_const++) {
		for (set<sail::Constant*>::iterator it2_const =
				op2_t.get_constants().begin();
				it2_const != op2_t.get_constants().end(); it2_const++) {
			constraint_ele* con_ele_oper1 = new constraint_ele(oper1_s,
					(*it1_const)->get_integer_value());
			constraint_ele* con_ele_oper2 = new constraint_ele(oper2_s,
					(*it2_const)->get_integer_value());
			ConstraintItem* con_item = NULL;
			if (first_f)
				con_item = new ConstraintItem();
			else
				con_item = con_i;
			con_item->add_to_contraint(con_ele_oper1);
			con_item->add_to_contraint(con_ele_oper2);
			if (is_bt_cmp_true(bt, (*it1_const)->get_integer_value(),
					(*it2_const)->get_integer_value()))
				two_br_con->add_to_then_branch(con_item);
			else
				two_br_con->add_to_else_branch(con_item);
		}
	}
}

void binop_operd1_const_operd2_ap(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& op1_t, target& op2_t, il::binop_type bt,
		TwoBranchConstraints*& two_br_con, bool first_f,
		ConstraintItem* con_i) {
	for (set<sail::Constant*>::iterator it1_const =
			op1_t.get_constants().begin();
			it1_const != op1_t.get_constants().end(); it1_const++) {
		for (set<AccessPath*>::iterator it2_other = op2_t.get_others().begin();
				it2_other != op2_t.get_others().end(); it2_other++) {
			constraint_ele* con_ele_oper1 = new constraint_ele(oper1_s,
					(*it1_const)->get_integer_value());
			constraint_ele* con_ele_oper2 = new constraint_ele(oper2_s,
					*it2_other);
			ConstraintItem* con_item = NULL;
			if (first_f)
				con_item = new ConstraintItem();
			else
				con_item = con_i;
			con_item->add_to_contraint(con_ele_oper1);
			con_item->add_to_contraint(con_ele_oper2);
			if (is_eq_bt(bt)) {
				if (bt == il::_EQ) {
					two_br_con->add_to_else_branch(con_item);
				} else {
					two_br_con->add_to_then_branch(con_item);
				}
			} else {
				two_br_con->add_to_then_branch(con_item);
				two_br_con->add_to_else_branch(con_item);
			}
		}
	}
}

void binop_operd1_ap_operd2_const(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& op1_t, target& op2_t, il::binop_type bt,
		TwoBranchConstraints*& two_br_con, bool first_f,
		ConstraintItem* con_i) {
	for (set<AccessPath*>::iterator it1_other = op1_t.get_others().begin();
			it1_other != op1_t.get_others().end(); it1_other++) {
		for (set<sail::Constant*>::iterator it2_const =
				op2_t.get_constants().begin();
				it2_const != op2_t.get_constants().end(); it2_const++) {
			constraint_ele* con_ele_oper1 = new constraint_ele(oper1_s,
					*it1_other);
			constraint_ele* con_ele_oper2 = new constraint_ele(oper2_s,
					(*it2_const)->get_integer_value());
			ConstraintItem* con_item = NULL;
			if (first_f)
				con_item = new ConstraintItem();
			else
				con_item = con_i;
			con_item->add_to_contraint(con_ele_oper1);
			con_item->add_to_contraint(con_ele_oper2);
			if (is_eq_bt(bt)) {
				if (bt == il::_EQ) {
					two_br_con->add_to_else_branch(con_item);
				} else {
					two_br_con->add_to_then_branch(con_item);
				}
			} else {
				two_br_con->add_to_then_branch(con_item);
				two_br_con->add_to_else_branch(con_item);
			}
		}
	}
}


void binop_operands_both_aps(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& op1_t, target& op2_t, il::binop_type bt,
		TwoBranchConstraints*& two_br_con, bool first_f,
		ConstraintItem* con_i) {
	for (set<AccessPath*>::iterator it1_other = op1_t.get_others().begin();
			it1_other != op1_t.get_others().end(); it1_other++) {
		for (set<AccessPath*>::iterator it2_other = op2_t.get_others().begin();
				it2_other != op2_t.get_others().end(); it2_other++) {
			constraint_ele* con_ele_oper1 = new constraint_ele(oper1_s,
					*it1_other);
			constraint_ele* con_ele_oper2 = new constraint_ele(oper2_s,
					*it2_other);
			ConstraintItem* con_item = NULL;
			if (first_f)
				con_item = new ConstraintItem();
			else
				con_item = con_i;
			con_item->add_to_contraint(con_ele_oper1);
			con_item->add_to_contraint(con_ele_oper2);
			if (is_eq_bt(bt)) {
				if (is_bt_cmp_ap_true(bt, *it1_other, *it2_other)) {
					two_br_con->add_to_then_branch(con_item);
				} else {
					two_br_con->add_to_else_branch(con_item);
				}
			} else {
				two_br_con->add_to_then_branch(con_item);
				two_br_con->add_to_else_branch(con_item);
			}
		}
	}
}

void adding_two_new_constraints(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& op1_t, target& op2_t, il::binop_type bt,
		TwoBranchConstraints*& two_br_con, bool first_f,
		ConstraintItem* con_i) {
	cout << "inside get_init_constraints " << endl;
	two_br_con = new TwoBranchConstraints(); //two_constraints
	if (op1_t.has_constant() && op2_t.has_constant()) {
		binop_operands_both_constants(oper1_s, oper2_s, op1_t, op2_t, bt,
				two_br_con, first_f, con_i);
	}

	if (op1_t.has_constant() && op2_t.has_other()) {
		binop_operd1_const_operd2_ap(oper1_s, oper2_s, op1_t, op2_t, bt,
				two_br_con, first_f, con_i);
	}

	if (op1_t.has_other() && op2_t.has_constant()) {
		binop_operd1_ap_operd2_const(oper1_s, oper2_s, op1_t, op2_t, bt,
				two_br_con, first_f, con_i);
	}

	if (op1_t.has_other() && op2_t.has_other()) {
		binop_operands_both_aps(oper1_s, oper2_s, op1_t, op2_t, bt, two_br_con,
				first_f, con_i);
	}

	cout << "check then branch condition == == " << endl
			<< two_br_con->get_then_branch_constraints()->to_string() << endl;
	cout << "check else branch condition == == " << endl
			<< two_br_con->get_else_branch_constraints()->to_string() << endl;
}

//////////////////////////////////////////////////////////////////////
void both_consts_one_exist(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& oper1_t, target& oper2_t, il::binop_type bt,
		ConstraintItem* con_item, TwoBranchConstraints*& two_br_con,
		bool oper1_exist) {
	set<sail::Constant*> const_set;
	if (oper1_exist) {
		const_set = oper2_t.get_constants();
		for (set<sail::Constant*>::iterator it1 = const_set.begin();
				it1 != const_set.end(); it1++) {
			constraint_ele* ele = new constraint_ele(oper2_s,
					(*it1)->get_integer_value());
			con_item->add_to_contraint(ele);
			if (is_bt_cmp_true(bt, con_item->get_value(oper1_s),
					(*it1)->get_integer_value())) {
				two_br_con->add_to_then_branch(con_item);
			} else {
				two_br_con->add_to_else_branch(con_item);
			}
		}
	} else { //operand2 exising
		const_set = oper1_t.get_constants();
		for (set<sail::Constant*>::iterator it1 = const_set.begin();
				it1 != const_set.end(); it1++) {
			constraint_ele* ele = new constraint_ele(oper1_s,
					(*it1)->get_integer_value());
			con_item->add_to_contraint(ele);
			if (is_bt_cmp_true(bt, (*it1)->get_integer_value(),
					con_item->get_value(oper2_s))) {
				two_br_con->add_to_then_branch(con_item);
			} else {
				two_br_con->add_to_else_branch(con_item);
			}
		}
	}
}

void both_aps_one_exist(sail::Symbol* oper1_s, sail::Symbol* oper2_s,
		target& oper1_t, target& oper2_t, il::binop_type bt,
		ConstraintItem* con_item, TwoBranchConstraints*& two_br_con,
		bool oper1_exist) {
	if (oper1_exist) {
		set<AccessPath*> aps = oper2_t.get_others();
		for (set<AccessPath*>::iterator it = aps.begin(); it != aps.end();
				it++) {
			constraint_ele* ele = new constraint_ele(oper2_s, *it);
			con_item->add_to_contraint(ele);
			if (is_eq_bt(bt)) {
				if (is_bt_cmp_ap_true(bt, con_item->get_ap(oper1_s), *it))
					two_br_con->add_to_then_branch(con_item);
				else
					two_br_con->add_to_else_branch(con_item);
			} else {
				two_br_con->add_to_then_branch(con_item);
				two_br_con->add_to_else_branch(con_item);
			}
		}
	} else {
		set<AccessPath*> aps = oper1_t.get_others();
		for (set<AccessPath*>::iterator it = aps.begin(); it != aps.end();
				it++) {
			constraint_ele* ele = new constraint_ele(oper1_s, *it);
			con_item->add_to_contraint(ele);
			if (is_eq_bt(bt)) {
				if (is_bt_cmp_ap_true(bt, *it, con_item->get_ap(oper2_s)))
					two_br_con->add_to_then_branch(con_item);
				else
					two_br_con->add_to_else_branch(con_item);
			} else {
				two_br_con->add_to_then_branch(con_item);
				two_br_con->add_to_else_branch(con_item);
			}

		}
	}
}
void operd1_const_operd2_ap_one_exist(sail::Symbol* oper1_s,
		sail::Symbol* oper2_s, target& oper1_t, target& oper2_t,
		il::binop_type bt, ConstraintItem* con_item,
		TwoBranchConstraints*& two_br_con, bool oper1_exist) {
	if (oper1_exist) {
		set<AccessPath*> aps = oper2_t.get_others();
		for (set<AccessPath*>::iterator it = aps.begin(); it != aps.end();
				it++) {
			constraint_ele* ele = new constraint_ele(oper2_s, *it);
			con_item->add_to_contraint(ele);
			if (is_eq_bt(bt)) { // == /!=
				if (bt == il::_EQ) {
					two_br_con->add_to_else_branch(con_item);
				} else
					two_br_con->add_to_then_branch(con_item);
			} else {
				two_br_con->add_to_then_branch(con_item);
				two_br_con->add_to_else_branch(con_item);
			}
		}
	} else {
		set<sail::Constant*> cons = oper1_t.get_constants();
		for (set<sail::Constant*>::iterator it = cons.begin(); it != cons.end();
				it++) {
			constraint_ele* ele = new constraint_ele(oper1_s,
					(*it)->get_integer_value());
			con_item->add_to_contraint(ele);
			if (is_eq_bt(bt)) { // == /!=
				if (bt == il::_EQ) {
					two_br_con->add_to_else_branch(con_item);
				} else
					two_br_con->add_to_then_branch(con_item);
			} else {
				two_br_con->add_to_then_branch(con_item);
				two_br_con->add_to_else_branch(con_item);
			}
		}
	}

}

void oper1_ap_oper2_const_one_exist(sail::Symbol* oper1_s,
		sail::Symbol* oper2_s, target& oper1_t, target& oper2_t,
		il::binop_type bt, ConstraintItem* con_item,
		TwoBranchConstraints*& two_br_con, bool oper1_exist) {
	if (oper1_exist) {
		set<sail::Constant*> oper2_value_set = oper2_t.get_constants();
		for (set<sail::Constant*>::iterator it1 = oper2_value_set.begin();
				it1 != oper2_value_set.end(); it1++) {
			constraint_ele* ele = new constraint_ele(oper2_s,
					(*it1)->get_integer_value());
			con_item->add_to_contraint(ele);
			if (is_eq_bt(bt)) { // == /!=
				if (bt == il::_EQ) {
					two_br_con->add_to_else_branch(con_item);
				} else
					two_br_con->add_to_then_branch(con_item);
			} else {
				two_br_con->add_to_then_branch(con_item);
				two_br_con->add_to_else_branch(con_item);
			}
		}
	} else {
		set<AccessPath*> aps = oper1_t.get_others();
		for (set<AccessPath*>::iterator it = aps.begin(); it != aps.end();
				it++) {
			constraint_ele* ele = new constraint_ele(oper1_s, *it);
			con_item->add_to_contraint(ele);
			if (is_eq_bt(bt)) { // == /!=
				if (bt == il::_EQ) {
					two_br_con->add_to_else_branch(con_item);
				} else
					two_br_con->add_to_then_branch(con_item);
			} else {
				two_br_con->add_to_then_branch(con_item);
				two_br_con->add_to_else_branch(con_item);
			}
		}
	}

}

void update_existing_constraints(Constraints* con, sail::Symbol* oper1_s,
		sail::Symbol* oper2_s, target& oper1_t, target& oper2_t,
		il::binop_type bt, TwoBranchConstraints*& two_br_con) {
	two_br_con = new TwoBranchConstraints();
	//conditions right before the comparable binop instruction;
	assert(con->has_constraints());
	set<ConstraintItem*> contents = con->get_constraints_content();
	set<ConstraintItem*>::iterator it = contents.begin();

	for (; it != contents.end(); it++) {
		ConstraintItem* con_item = *it;
		if (con_item->has_two_symbols(oper1_s, oper2_s)) { //found two symbols, then compare it
			cout << endl << "In containing two operands " << endl;
			if ((con_item->get_which(oper1_s) == VALUE_CONTENT)
					&& (con_item->get_which(oper2_s) == VALUE_CONTENT)) {
				if ((is_bt_cmp_true(bt, con_item->get_value(oper1_s),
						con_item->get_value(oper2_s)))) { //not satisfying the bt compare
					two_br_con->add_to_then_branch(con_item);
				} else {
					two_br_con->add_to_else_branch(con_item);
				}
			} else if (con_item->get_which(oper1_s)
					!= con_item->get_which(oper2_s)) {
				if (is_eq_bt(bt)) {
					if (bt == il::_EQ) {
						two_br_con->add_to_else_branch(con_item);
					} else {
						two_br_con->add_to_then_branch(con_item);
					}
				} else {
					two_br_con->add_to_then_branch(con_item);
					two_br_con->add_to_else_branch(con_item);
				}
			} else if ((con_item->get_which(oper1_s) == AP_CONTENT)
					&& (con_item->get_which(oper2_s) == AP_CONTENT)) {
				if (is_eq_bt(bt)) {
					if (is_bt_cmp_ap_true(bt, con_item->get_ap(oper1_s),
							con_item->get_ap(oper2_s))) {
						two_br_con->add_to_then_branch(con_item);
					} else {
						two_br_con->add_to_else_branch(con_item);
					}

				}

			}
		} else if (con_item->has_symbol(oper1_s)) {
			cout << endl << "In containing oper1  " << endl;
			if (con_item->get_which(oper1_s) == VALUE_CONTENT) { //op1 is constant
				if (!oper2_t.has_no_value()) { //has content;
					if (oper2_t.has_constant()) {
						both_consts_one_exist(oper1_s, oper2_s, oper1_t,
								oper2_t, bt, con_item, two_br_con, true);
					}

					if (oper2_t.has_other()) { //has ap
						operd1_const_operd2_ap_one_exist(oper1_s, oper2_s,
								oper1_t, oper2_t, bt, con_item, two_br_con,
								true);
					}
				}
			} else if (con_item->get_which(oper1_s) == AP_CONTENT) { //this time the ope1 ele is ap type
				if (!oper2_t.has_no_value()) {
					if (oper2_t.has_constant()) {
						oper1_ap_oper2_const_one_exist(oper1_s, oper2_s,
								oper1_t, oper2_t, bt, con_item, two_br_con,
								true);
					}
					if (oper2_t.has_other()) {
						both_aps_one_exist(oper1_s, oper2_s, oper1_t, oper2_t,
								bt, con_item, two_br_con, true);
					}
				}

			}
		} else if (con_item->has_symbol(oper2_s)) {
			cout << endl << "In containing oper2  " << endl;
			if (con_item->get_which(oper2_s) == VALUE_CONTENT) { //op1 is constant
				if (!oper1_t.has_no_value()) { //has content;
					if (oper1_t.has_constant()) {
						both_consts_one_exist(oper1_s, oper2_s, oper1_t,
								oper2_t, bt, con_item, two_br_con, false);
					}

					if (oper1_t.has_other()) { //has ap
						oper1_ap_oper2_const_one_exist(oper1_s, oper2_s,
								oper1_t, oper2_t, bt, con_item, two_br_con,
								false);
					}
				}
			} else if (con_item->get_which(oper2_s) == AP_CONTENT) { //this time the ope1 ele is ap type
				if (!oper1_t.has_no_value()) {
					if (oper1_t.has_constant()) {
						operd1_const_operd2_ap_one_exist(oper1_s, oper2_s,
								oper1_t, oper2_t, bt, con_item, two_br_con,
								false);
					}
					if (oper1_t.has_other()) {
						both_aps_one_exist(oper1_s, oper2_s, oper1_t, oper2_t,
								bt, con_item, two_br_con, false);
					}
				}
			}
		} else { //don't have any symbols
			adding_two_new_constraints(oper1_s, oper2_s, oper1_t, oper2_t, bt,
					two_br_con, false, con_item);
		}

	}
	cout << "check then branch condition == == " << endl
			<< two_br_con->get_then_branch_constraints()->to_string() << endl;
	cout << "check else branch condition == == " << endl
			<< two_br_con->get_else_branch_constraints()->to_string() << endl;
}

bool is_bt_cmp_true(il::binop_type bt, long int oper1_value,
		long int oper2_value) {
	switch (bt) {
	case il::_LT: { // "<"
		return (oper1_value < oper2_value);
	}
	case il::_LEQ: { // "<="
		return (oper1_value <= oper2_value);
	}
	case il::_GT: { // ">"
		return (oper1_value > oper2_value);
	}
	case il::_GEQ: { // ">="
		return (oper1_value >= oper2_value);
	}
	case il::_EQ: { // "=="
		return (oper1_value == oper2_value);
	}
	case il::_NEQ: { // "!="
		//cout << " oper1_value " << oper1_value << endl;
		//cout << " oper2_value " << oper2_value << endl;
		return (oper1_value != oper2_value);
	}
	default: {
		cout << "binop bt " << bt << endl;
		assert(false);
	}

	}
}

bool is_bt_cmp_ap_true(il::binop_type bt, AccessPath* ap1, AccessPath* ap2) {
	switch (bt) {
	case il::_EQ: {
		return (ap1 == ap2);
	}
		break;
	case il::_NEQ: {
		return (ap1 != ap2);
	}
		break;
	default:
		return false;
	}
}

bool is_eq_bt(il::binop_type bt) {
	switch (bt) {
	case il::_EQ:
	case il::_NEQ:
		return true;
	default:
		return false;
	}
}


bool comparable_bt(il::binop_type bt) {
	switch (bt) {
	case il::_LT:
	case il::_LEQ:
	case il::_GT:
	case il::_GEQ:
	case il::_EQ:
	case il::_NEQ:
		return true;
	default:
		return false;
	}
}


bool binop_type_need_to_process(il::binop_type bt) {
	switch (bt) {
	case il::_PLUS:
	case il::_MINUS:
	case il::_MULTIPLY:
	case il::_DIV:
	case il::_MOD:
	case il::_BITWISE_XOR:
	case il::_BITWISE_AND:
	case il::_BITWISE_OR:
	case il::_LEFT_SHIFT:
	case il::_RIGHT_SHIFT:
		return true;
	default: {
		return false;
	}

	}
}

void get_connect_blockids(sail::Block* begin, sail::Block* end,
		vector<int>& block_ids) {
	if (begin != end) {
		set<sail::CfgEdge*> succs = begin->get_successors();
		for (set<sail::CfgEdge*>::iterator it = succs.begin();
				it != succs.end(); it++) {
			sail::Block* target = (*it)->get_target();
			if (target->get_block_id() != end->get_block_id()) {
				block_ids.push_back(target->get_block_id());
				cout << "inserting " << target->get_block_id() << endl;
				get_connect_blockids(target, end, block_ids);
			}
		}
	}
}

int get_index_of_label(vector<sail::Instruction*>*& body, sail::Label* l) {
	for (unsigned int i = 0; i < body->size(); i++) {
		if (body->at(i)->get_instruction_id() == sail::SAIL_LABEL) {
			if (((sail::Label*) body->at(i)) == l) {
				cout << "finding the label " << l->to_string()
						<< "  at index == " << i << endl;
				return i;
			}
		}
	}
	cout << "can't find the label " << l->to_string() << endl;
	return -1;
}

void write_to_dotfiles(const Identifier& id,
		const string& append_str,
		const string& output_folder, MemGraph* mg) {
	//cerr << "called for " << append_str << endl;
	string output_dir = output_folder;
	if (output_dir.size() == 0 || output_dir[output_dir.size() - 1] != '/')
		output_dir += "/";

	FileIdentifier out_fi(output_dir);

	string expanded_dir = out_fi.to_string('/');

	string identifier = id.to_string('#');

	string file_name = get_legal_filename(expanded_dir, identifier);
	file_name += append_str;
	file_name += ".dot";
	cout << "file_name ?? " << file_name << endl;

	ofstream myfile;
	myfile.open (file_name.c_str(), std::ofstream::out | ios::trunc);
	//cout << "check 1111111111111111111111111111111111 " << endl;

	myfile << mg->to_dotty() << endl;
	//cout << "check 222222222222222222222222222222222 " << endl;
	myfile.close();

}

void print_inst(sail::Instruction* ins) {
	assert(ins != NULL);
	cout << ins->to_string() << endl;
}



/*
 * feed with arithvalue or function ap type, each ap(term) is terminal ap;
 */
void collect_arith_fun_aps(AccessPath* cur_ap,
		vector<AccessPath*>& aps, bool& arg_flag) {
	Term* t = NULL;
	if (cur_ap->get_ap_type() == AP_ARITHMETIC) {
		ArithmeticValue* av = static_cast<ArithmeticValue*>(cur_ap);
		t = av;
	} else {
		FunctionValue* fv = static_cast<FunctionValue*>(cur_ap);
		FunctionTerm* ft = fv;
		t = ft;
	}
	assert(t != NULL );
	//cout << "term type is ==  " << t->get_term_type() << endl;
	collect_access_paths(t, aps);
	arg_flag = access_paths_has_argument(aps);

//	if(CONSIDER_ARITH_OF_ALLOC)
//		arg_flag = access_paths_has_alloc(aps);
}


bool access_paths_has_argument(vector<AccessPath*>& aps) {
	vector<AccessPath*>::iterator it = aps.begin();
	for (; it != aps.end(); it++) {
		AccessPath* ap = *it;
		assert((ap->get_ap_type() == AP_VARIABLE)
				||(ap->get_ap_type() == AP_CONSTANT));
		if(ap->get_ap_type() == AP_VARIABLE){
			Variable* var = (Variable*)ap;
			if(var->is_argument())
				return true;
		}
	}
	return false;
}


bool access_paths_has_alloc(vector<AccessPath*>& aps){
	auto it = aps.begin();

	for(; it != aps.end(); it ++){
		AccessPath* ap = *it;
		assert((ap->get_ap_type() == AP_VARIABLE)
						||(ap->get_ap_type() == AP_CONSTANT));

		if(ap->get_ap_type() == AP_VARIABLE){
			Variable* var = (Variable*)ap;
			if(var->get_original_var()->has_point_to_alloc())
				return true;
			}
	}
	return false;
}


void collect_access_paths(Term* t, vector<AccessPath*>& aps) {
	assert(t != NULL);
	switch (t->get_term_type()) {
	case CONSTANT_TERM: {
		assert(t->is_specialized());
		aps.push_back(AccessPath::to_ap(t));
		break;
	}
	case VARIABLE_TERM: {
		aps.push_back(AccessPath::to_ap(t));
	}
		break;
	case FUNCTION_TERM: {
		FunctionValue* fv = (FunctionValue*) t;
		FunctionTerm* fn = fv;

		vector<Term*> terms = fn->get_args();
		for (vector<Term*>::iterator it = terms.begin(); it != terms.end();
				it++) {
			collect_access_paths(*it, aps);
		}
		break;
	}
	case ARITHMETIC_TERM: {
		ArithmeticTerm* at = (ArithmeticTerm*) t;
		const map<Term*, long int>& elems = at->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for (; it != elems.end(); it++) {
			Term* cur = it->first;
			collect_access_paths(cur, aps);
		}

		long int constant = at->get_constant();
		if (constant != 0) {
			ConstantValue* const_value = ConstantValue::make(constant);
			ConstantTerm* ct = const_value;
			collect_access_paths(ct, aps);
		}

		break;
	}
	default:
		assert(false);

	}
}


sail::Symbol* get_variable(Term* t){
	assert(t != NULL);
	//cout << "term is " << t->to_string() << endl;
	sail::Symbol* s = NULL;
	term_type tt = t->get_term_type();
	if(tt == CONSTANT_TERM){
		ConstantTerm* ct = (ConstantTerm*)(t);
		ConstantValue* cv = (ConstantValue*)ct;
		sail::Constant* c = new sail::Constant(cv->get_constant(),true, 32);
		s = c;
	}else if(tt == VARIABLE_TERM){
		VariableTerm* vt = (VariableTerm*)(t);
		Variable* v = (Variable*)vt;
		//cout << "variable is " << v->to_string() << endl;
		s = v->get_original_var();
	}else{
		//some how FieldSelection also could be term
		cout << "term_type is " << tt << endl;
		if(AccessPath::to_ap(t)->get_ap_type() == AP_FIELD){
			FieldSelection* fs = (FieldSelection*)(AccessPath::to_ap(t));
			AccessPath* outerest_ap = ((Deref*)fs->get_inner())->get_inner();
			if(outerest_ap->get_ap_type() == AP_VARIABLE)
				s = ((Variable*)(outerest_ap))->get_original_var();
			else
				assert(false);
		}
		//assert(false);
	}
	return s;
}


void get_unop_code(long int value, il::unop_type& unop){
	switch(value){
	case -1:
		break;
	case 1:
		break;
	case 6:
		break;
	case 13:
		break;
	case 14:
		break;
	default:
		break;
	}
}

void get_binop_code(long int value, il::binop_type& binop){
	switch (value) {
	case -2:
		binop = il::_MINUS;
		break;
	case -1:
		binop = il::_PLUS;
		break;
	case 1:
		binop = il::_PLUS;
		break;
	case 2:
		binop = il::_BITWISE_OR;
		break;
	case 3:
		binop = il::_BITWISE_XOR;
		break;
	case 4:
		binop = il::_LEFT_SHIFT;
		break;
	case 5:
		binop = il::_RIGHT_SHIFT;
		break;
	case 6:
		binop = il::_MOD;
		break;
	case 13:
		binop = il::_MULTIPLY;
		break;
	case 14:
		binop = il::_DIV;
		break;
	default: {
		cout << "binop type " << value << endl;
		assert(false);
	}
		break;
	}
}



void _mkdir(const char *dir)
{
        char tmp[5560];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, S_IRWXU | S_IRWXG | S_IRWXO);
                        *p = '/';
                }
        mkdir(tmp, S_IRWXU);
}

string replace_illegal_characters(const string& s)
{
	string res;
	for(unsigned int i=0; i < s.size(); i++)
	{
		char c = s[i];
		if(c=='/') res += "#";
		else res += c;
	}
	return res;

}



string get_legal_filename(string  directory, const string & _identifier)
{
	if(directory.size() == 0 || directory[directory.size()-1] != '/')
		directory+='/';
	string identifier = replace_illegal_characters(_identifier);

	if(identifier.size() >(MAX_FILENAME_SIZE-MIN_FILNAME_SIZE))
	{
		string cur;
		while(identifier.size() > (MAX_FILENAME_SIZE-MIN_FILNAME_SIZE))
		{
			cur = identifier.substr(0, (MAX_FILENAME_SIZE-MIN_FILNAME_SIZE));
			identifier = identifier.substr((MAX_FILENAME_SIZE-MIN_FILNAME_SIZE));

			if(identifier.size() < MIN_FILNAME_SIZE) {
				cur += identifier;
				identifier = "";
			}
			directory+=cur + "/";
			if(identifier != "") _mkdir(directory.c_str());



		}
		directory+= identifier;
		if(directory[directory.size()-1] == '/')
			return directory.substr(0, directory.size()-1);
		return directory;


	}
	return directory + identifier;

}



void open_file(ofstream& ofs, const string& file) {
  ofs.open (file, std::ofstream::out | std::ofstream::app);
}



void print_ap_type(AccessPath* ap) {

	cerr << "ap type is :::::::: ";

	switch (ap->get_ap_type()) {
	case AP_NON_AP: {
		cerr << "AP_NON_AP" << endl;
	}
		break;
	case AP_ARITHMETIC: {
		cerr << "AP_ARITHMETIC" << endl;
	}
		break;
	case AP_ARRAYREF: {
		cerr << "AP_ARRAYREF" << endl;
	}
		break;
	case AP_CONSTANT: {
		cerr << "AP_CONSTANT" << endl;
	}
		break;
	case AP_DEREF: {
		cerr << "AP_DEREF" << endl;
	}
		break;
	case AP_FIELD: {
		cerr << "AP_FIELD" << endl;
	}
		break;
	case AP_FUNCTION: {
		cerr << "AP_FUNCTION" << endl;
	}
		break;
	case AP_VARIABLE: {
		cerr << "AP_ARITHMETIC" << endl;
	}
		break;
	case AP_ADDRESS: {
		cerr << "AP_ADDRESS" << endl;
	}
		break;
	case AP_ALLOC: {
		cerr << "AP_ALLOC" << endl;
	}
		break;
	case AP_UNMODELED: {
		cerr << "AP_UNMODELED" << endl;
	}
		break;
	case AP_INDEX: {
		cerr << "AP_INDEX" << endl;
	}
		break;
	case AP_COUNTER: {
		cerr << "AP_COUNTER" << endl;
	}
		break;
	case AP_ADT: {
		cerr << "AP_ADT" << endl;
	}
		break;
	case AP_STRING: {
		cerr << "AP_STRING" << endl;
	}
		break;
	case AP_NIL: {
		cerr << "AP_NIL" << endl;
	}
		break;
	case AP_PROGRAM_FUNCTION: {
		cerr << "AP_PROGRAM_FUNCTION" << endl;
	}
		break;
	case AP_TYPE_CONSTANT: {
		cerr << "AP_TYPE_CONSTANT" << endl;
	}
		break;
	case AP_END: {
		cerr << "AP_END" << endl;
	}
		break;
	default:
		assert(false);
	}

}

//para should greater than or equal to  arg;
bool compatible_for_function_call(il::type* arg,
		il::type* para){
if(arg == para)
	return true;

if(para == il::type::get_type_from_name(JAVA_OBJECT_POINTER))
	return true;



if(para->is_pointer_type()){

	if(!arg->is_pointer_type())
		return false;

	il::pointer_type* para_pt = static_cast<il::pointer_type*>(para);
	il::pointer_type* arg_pt = static_cast<il::pointer_type*>(arg);


	il::type* inner_para = para_pt->get_inner_type();
	il::type* inner_arg = arg_pt->get_inner_type();

	if(inner_para->is_record_type()
			&& (inner_arg->is_record_type()))
	{

		il::record_type* arg_rt = static_cast<il::record_type*>(inner_arg);
		il::record_type* para_rt = static_cast<il::record_type*>(inner_para);

		const map<int,il::record_type*>& bases = arg_rt->get_bases();
		for(auto it = bases.begin(); it  != bases.end(); it++){
			il::record_type* bt = it->second;
			if(bt == il::type::get_type_from_name(JAVA_OBJECT)){
				continue;
			}else{
				if(bt == para_rt){
					return true;
				}

			}
		}
		return false;


	}else{

		cerr << "Arg is " << arg->to_string() << endl << "	"
				"&& 	Para is " << para->to_string() << endl;
		cerr << "Can't match the function signature ! " << endl;

		return false;
	}

}else{
//	cerr << "Arg is " << arg->to_string() << endl << "	"
//					"&& 	Para is " << para->to_string() << endl;
//	cerr << "Can't match the function signature ! " << endl;
//	assert(false);
	return true;
}


}


//bool find_rhs_symbols(sail::Instruction* inst, set<one_side*>& symbols){
//	switch (ins->get_instruction_id()) {
//
//		case sail::CAST: {
//			sail::Cast* cast = (sail::Cast*) ins;
//			if(symbols.count( cast->get_rhs()) > 0)
//				return true;
//			return false;
//		}
//
//
//		case sail::ASSIGNMENT: {
//
//			sail::Assignment* assi = (sail::Assignment*) ins;
//
//			if(symbols.count(assi->get_rhs()) > 0)
//				return true;
//
//			return false;
//		}
//		case sail::STORE: {
//			sail::Store* s = (sail::Store*) ins;
//
//			if(symbols.count(s->get_rhs()) > 0)
//				return true;
//
//			return false;
//		}
//
//		case sail::LOAD: {
//
//			sail::Load* l = (sail::Load*) ins;
//
//			//make sure what is rhs?
//
//		}
//
//
//		case sail::BINOP: {
//
//			sail::Binop* binop = (sail::Binop*) ins;
//
//			if (symbols.count(binop->get_first_operand()) == 0)
//				return false;
//
//			if(symbols.count(binop->get_second_operand()) == 0)
//				return false;
//
//			return true;
//		}
//
//
//		case sail::UNOP: {
//
//			sail::Unop* unop = (sail::Unop*) ins;
//
//			if (symbols.count(unop->get_operand()) > 0)
//				return true;
//
//			return false;
//		}
//		default: {
//			//used to catch other type of instructions
//			cerr << "INS:: " << ins->to_string()
//					<< "   shouldn't inside the in_rule container! " << endl<<endl;
//			assert(false);
//		}
//			return;
//
//		}
//}
