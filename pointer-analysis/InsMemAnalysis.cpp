#include "InsMemAnalysis.h"
#include "access-path.h"
#include "GlobalAssert.h"
//#include "StaticAssertBuilder.h"
#include "MtdInstsProcessor.h";

#include "CallManager.h"

#define CHECK_FOR_SINGLE_INS false

//CallManager* InsMemAnalysis::cm = NULL;

InsMemAnalysis::InsMemAnalysis(sail::Instruction* ins, MtdInstsProcessor* mp, int i){

	instruction_number = i;
	this->ins = ins;
	this->mp = mp;

}



InsMemAnalysis::~InsMemAnalysis() {

}


bool InsMemAnalysis::need_process() const {

	bool flag = false;
	switch (ins->get_instruction_id()) {
	case sail::ASSIGNMENT:
	case sail::LOAD:
	case sail::STORE:
	case sail::STATIC_ASSERT:
	case sail::CAST:
		flag = true;
		break;
	case sail::ADDRESS_STRING:{ //rule out monitor-enter!
		sail::AddressString* addr_str = (sail::AddressString*)ins;
		il::string_const_exp* str_exp =  addr_str->get_rhs();
		if((str_exp->get_string() != "monitor-enter")
			&&(str_exp->get_string() != "monitor-exit"))
			flag = true;
	}break;
	case sail::UNOP:{
		il::unop_type un_t = ((sail::Unop*)ins)->get_unop();
		if(un_t != il::_LOGICAL_NOT)
			flag = true;
	}break;
	case sail::BINOP: {
		flag = binop_type_need_to_process(((sail::Binop*) ins)->get_binop());
	}break;
	case sail::FUNCTION_CALL: {
		sail::FunctionCall* fc = (sail::FunctionCall*) ins;
		if (fc->is_allocator()) {
			flag = true;
		}else{
			cout << "@@@@@@@@@@ *********** "
				" ******* store_into_unresolved_fc_map " << endl <<
			fc->to_string() << endl << endl;
			mp->store_into_unresolved_fc_map(fc);
		}
	}
		break;

	case sail::BRANCH:
	case sail::INSTANCEOF:
	case sail::JUMP:
	case sail::SAIL_LABEL:
	case sail::EXCEPTION_RETURN:
		break;
	default:
		assert(false);
		break;
	}
	return flag;
}




void InsMemAnalysis::inprecise_ins_analysis(const analysis_type& at) {


	if (need_process()) {

		//for the purpose of walk around instructions containing invalid type
		//do not process, because it is inside the exception blocks;
		if(inst_contain_invalid_type(this->ins)){
			cerr << "ins  " << ins->to_string() <<
					"	Inside exception handle branch!" << endl;
			assert(false);
		}


		//inst not inside static_assert and not processed before
		//static_assert should be considered to check

		if (PROCESS_STAITC_ASSERT) {
			//if (need_to_check_before_bbs) {
			if (at == CHECK_BEFORE_BBS) {
				assert(mp->get_static_assert_builder() != NULL);

				if (mp->get_static_assert_builder()->get_global_assert() != NULL)
					mp->get_static_assert_builder()->get_global_assert()->check_must_before_static_ins(
							this->ins);
			}
		}


		//3 ways both doing the thing
		vector<symbol_ap_mem*>* symbol_ap = get_access_paths();


		//lhs symbol and ap
		sail::Symbol* lhs_symbol = symbol_ap->at(0)->s;
		AccessPath* lhs_ap = symbol_ap->at(0)->p;



		//rhs symbol and ap
		sail::Symbol* rhs_symbol = symbol_ap->at(1)->s;

		if (0) {
			if (rhs_symbol != NULL)
				cout << "rhs_symbol is " << rhs_symbol->to_string() << endl;
			else
				cout << "rhs_symbol is NULL " << endl;
		}

		AccessPath* rhs_ap = symbol_ap->at(1)->p;


		//the rhs_op2 may not be existing, but if is existing, please init it!!!
		sail::Symbol* rhs_symbol_op2 = NULL;
		AccessPath* rhs_ap_op2 = NULL;


		if (ins->get_instruction_id() == sail::ADDRESS_STRING) {

			assert(symbol_ap->size() == 2);
			set_nodes_for_address_string(lhs_ap, rhs_ap, lhs_symbol);

		} else if (ins->get_instruction_id() == sail::FUNCTION_CALL) {

			if (rhs_symbol == NULL) {

				if (rhs_ap->get_ap_type() == AP_ALLOC) { //alloc memory
					assert(symbol_ap->size() == 3);

					rhs_symbol_op2 = symbol_ap->at(2)->s;
					rhs_ap_op2 = symbol_ap->at(2)->p;

					set_nodes_for_alloc_three_symbols(lhs_ap, rhs_ap,
							rhs_ap_op2, lhs_symbol, rhs_symbol_op2);

					//collect deref(alloc)
					assert(rhs_ap->get_ap_type() == AP_ALLOC);
					cout << "111111111111111111111  " << endl;
					mp->fill_alloc_context_controller(lhs_ap,rhs_ap, (sail::FunctionCall*)ins);
					cout << "22222222222222222222  " << endl;
				}else{ //other functioncall

					cout << "Process normal function call " << endl;
					//process_function_call();

				}
			}
		} else if (ins->get_instruction_id() != sail::BINOP) { //at most two symbol_ap_mem

			assert(lhs_ap->get_ap_type() == AP_VARIABLE);
			assert(
					(rhs_ap->get_ap_type() == AP_VARIABLE) ||(rhs_ap->get_ap_type() == AP_CONSTANT));

			set_nodes_for_symbols(lhs_ap, rhs_ap, lhs_symbol, rhs_symbol);


		} else { // binop
			if (symbol_ap->size() == 2) { //this is for __temp = array + variable(offset);
				//cout << "this is the reference to array ! " << endl;
				set_nodes_for_symbols(lhs_ap, rhs_ap, lhs_symbol, rhs_symbol);

			} else { //real binop, not related to array

				//cout << "|| || INS " << ins->to_string() << endl;
				rhs_symbol_op2 = symbol_ap->at(2)->s;
				rhs_ap_op2 = symbol_ap->at(2)->p;
				set_nodes_for_three_symbols(lhs_ap, rhs_ap, rhs_ap_op2,
						lhs_symbol, rhs_symbol, rhs_symbol_op2);
			}
		}



		///updating for " = "

		mp->get_mem_graph()->update_graph_for_ins(lhs_ap, rhs_ap,
				rhs_ap_op2, ins);



		//subset constraint
		//updating other rules && adding current rules (order can't change)
		if ((at == CHECK_BEFORE_BBS) || (at == BASIC)){

			this->mp->add_to_in_rules(ins);
		}


		if (CHECK_FOR_SINGLE_INS)
			mp->get_mem_graph()->to_dotty();




		//free space;
		for (vector<symbol_ap_mem*>::iterator it = symbol_ap->begin();
				it != symbol_ap->end(); it++)
			delete *it;
		delete symbol_ap;

	}
}

void InsMemAnalysis::set_nodes_for_symbols(AccessPath* lhs_ap,
		AccessPath* rhs_ap, sail::Symbol* lhs, sail::Symbol* rhs) {
	set_node_for_symbol(lhs_ap, lhs, false);
	set_node_for_symbol(rhs_ap, rhs, true);
}

void InsMemAnalysis::set_nodes_for_address_string(AccessPath* lhs_ap,
		AccessPath* rhs_ap, sail::Symbol* lhs_s) {
	set_node_for_symbol(lhs_ap, lhs_s, false);
	set_node_for_symbol(rhs_ap, NULL, false);
}

void InsMemAnalysis::set_nodes_for_three_symbols(AccessPath* lhs_ap,
		AccessPath* rhs_ap_op1, AccessPath* rhs_ap_op2,
		sail::Symbol* lhs_symbol, sail::Symbol* rhs_op1_symbol,
		sail::Symbol* rhs_op2_symbol) {
	set_node_for_symbol(lhs_ap, lhs_symbol, false);
	set_node_for_symbol(rhs_ap_op1, rhs_op1_symbol, true);
	set_node_for_symbol(rhs_ap_op2, rhs_op2_symbol, true);
}

void InsMemAnalysis::set_nodes_for_alloc_three_symbols(AccessPath* lhs_ap,
		AccessPath* allo_ap, AccessPath* size_ap, sail::Symbol* lhs_s,
		sail::Symbol* size_s) {
	cout << "== calling set_nodes_for_alloc_three_symbols() " << endl;
	set_node_for_symbol(lhs_ap, lhs_s, false);
	cout << "AAAAAAAAAAAAAAAAAAAAAA " << endl;
	set_node_for_symbol(allo_ap, NULL, true);
	cout << "BBBBBBBBBBBBBBBBBBBBBB " << endl;
	set_node_for_symbol(size_ap, size_s, true);
	cout << "CCCCCCCCCCCCCCCCCCCC " << endl;
	cout <<  "== end of set_nodes_for_alloc_three_symbols() " << endl;
}

//toppest node;
void InsMemAnalysis::set_node_for_symbol(AccessPath* ap, sail::Symbol* s,
		bool rhs_flag) {
	if(0)
		cout << "|||||||||||||||| set_node_for_symbol()  for function " <<
			this->mp->get_function()->get_identifier().to_string() << endl;


	assert(ap != NULL);

	AccessPath* print_ap = ap;
	bool find_location = false;
	bool variable_flag = false;
	sail::Variable* var = NULL;
	bool argument_flag = false;

	//making node for alloc
	if (s == NULL) {
		if (ap->get_ap_type() == AP_ALLOC) { //making deref for it
			cout << "Case AP_ALLOC " <<endl;
			MemNode* alloc_node = mp->get_mem_graph()->get_memory_node(ap);
			mp->get_mem_graph()->add_edge_to_next_layer(alloc_node, ap);
			return;
		} else if (ap->get_ap_type() == AP_STRING) {
			cout << "Case AP_STRING " <<endl;
			MemNode* string_literal_node =
					mp->get_mem_graph()->get_memory_node(ap);

			mp->get_mem_graph()->add_edge_to_next_layer(string_literal_node,
					ap);
			return;
		} else { //funtioncall
			cout
					<< "LOL encountering funtioncall exit, impossible here, find why!"
					<< endl;
			assert(false);
			return;
		}
	}

	assert(s != NULL);
	if (s->is_variable()) {
		var = static_cast<sail::Variable*>(s);
		variable_flag = true;
		assert(var != NULL);
		if (var->is_argument())
			argument_flag = true;
	}

	if (mp->get_mem_graph()->has_location(ap))
		find_location = true;
	else { //do not have a location

		if ((variable_flag) && (rhs_flag) && (!argument_flag)) {
			if (0) {
				cout << "Var Address == " << var << endl;
				cout << var->to_string() << "do not have a location !" << endl
						<< "Making an unknown init for its target because of the flow insensitive!! "
						<< endl;
			}

			MemNode* rhs_node = mp->get_mem_graph()->get_memory_node(ap);
			assert(rhs_node != NULL);
			if (0)
				cout << " == making toppest level node ::: "
						<< rhs_node->to_string() << "whose representative is "
						<< ap->to_string() << endl;

		} else { //create a node for a constant, for a lhs variable, for parameter
			MemNode* variable_type_node = mp->get_mem_graph()->get_memory_node(
					ap);

			assert(variable_type_node != NULL);
			if (0)
				cout << "          == making toppest level MemNode  :::"
						<< variable_type_node->to_string()
						<< " whose representative is " << ap->to_string()
						<< endl;
			//do init the deref of argument;
			if ((argument_flag) || (!variable_flag)) //argument or lhs variable or rhs constant
				mp->get_mem_graph()->add_edge_to_next_layer(variable_type_node,
						ap);

			if (0)
				cout
						<< "          == end of building everything for making MemNode ::: "
						<< "whose representative is  " << print_ap->to_string()
						<< endl << endl;
		}

		if (mp->get_mem_graph()->has_location(ap))
			find_location = true;
	}
	assert(find_location);
}



vector<symbol_ap_mem*>* InsMemAnalysis::get_access_paths() {

	vector<symbol_ap_mem*>* symbol_aps = new vector<symbol_ap_mem*>();

	switch (ins->get_instruction_id()) {

	case sail::ASSIGNMENT: {
		get_assignment_aps(*symbol_aps);
	}
		break;

	case sail::UNOP: {
		sail::Unop* unop = static_cast<sail::Unop*>(ins);
		il::unop_type un_t = unop->get_unop();
		cout << "un_t is " << un_t << endl;
		if (un_t == il::_NEGATE) {
			get_unop_aps(*symbol_aps);
		} else if (un_t == il::_LOGICAL_NOT) {
			cout << "BREAK!" << endl;
			assert(false);
			break;
		} else {
			cout << "UNOP type is " << un_t << endl;
			assert(false);
		}
	}
		break;

	case sail::BINOP: { //could set 2 or 3 symbol_ap_mem; (2 for vi = array + offset);
		get_binop_aps(*symbol_aps);
	}
		break;

	case sail::LOAD: {
		get_load_aps(*symbol_aps);
	}
		break;

	case sail::STORE: {
		get_store_aps(*symbol_aps);
	}
		break;

	case sail::ADDRESS_STRING: {
		get_address_string_aps(*symbol_aps);
	}
		break;

	case sail::CAST: {
		get_cast_aps(*symbol_aps);
	}
		break;

	case sail::FUNCTION_CALL: {
		sail::FunctionCall* fc = (sail::FunctionCall*) this->ins;
		if (fc->is_allocator()) {
			get_memory_alloc_aps(*symbol_aps);
			break;
		} else if (fc->is_constructor()) {

		} else {

//			cout << "encountering functioncall " << endl;
//			exit(1);
//			get_function_call_aps(*symbol_aps);
		}
	}
		break;

//	case sail::BRANCH: {
//		assert(false);
//	}
//		break;
//
//	case sail::INSTANCEOF: {
//		get_instanceof_aps(*symbol_aps);
//	}
//		break;
//
//	case sail::STATIC_ASSERT:
//	case sail::JUMP:
//	case sail::SAIL_LABEL:
//	case sail::EXCEPTION_RETURN:
//	case sail::BRANCH:
//		break;
	default:
		assert(false);
		break;
	}
	return symbol_aps;
}

void InsMemAnalysis::get_assignment_aps(vector<symbol_ap_mem*>& symbol_ap) {
	sail::Symbol* rhs = ((sail::Assignment*) this->ins)->get_rhs();

	//AccessPath* rhs_ap = AccessPath::get_ap_from_symbol(rhs);
	AccessPath* rhs_ap = get_ap_from_symbol(rhs, mp);
	sail::Variable* lhs = ((sail::Assignment*) this->ins)->get_lhs();
	//AccessPath* lhs_ap = AccessPath::get_ap_from_symbol(lhs);
	AccessPath* lhs_ap = get_ap_from_symbol(lhs, mp);
	symbol_ap.push_back(build_symbol_ap_mem(lhs, lhs_ap));
	symbol_ap.push_back(build_symbol_ap_mem(rhs, rhs_ap));
}

void InsMemAnalysis::get_unop_aps(vector<symbol_ap_mem*>& symbol_ap) {
	sail::Unop* ins = static_cast<sail::Unop*>(this->ins);
	sail::Variable* lhs = ins->get_lhs();
	sail::Symbol* rhs = ins->get_operand();
	//AccessPath* lhs_ap = AccessPath::get_ap_from_symbol(lhs);
	AccessPath* lhs_ap = get_ap_from_symbol(lhs, mp);
	//AccessPath* rhs_ap = AccessPath::get_ap_from_symbol(rhs);
	AccessPath* rhs_ap = get_ap_from_symbol(rhs, mp);
	symbol_ap.push_back(build_symbol_ap_mem(lhs, lhs_ap));
	symbol_ap.push_back(build_symbol_ap_mem(rhs, rhs_ap));
}

void InsMemAnalysis::get_binop_aps(vector<symbol_ap_mem*>& symbol_ap) {
	//added 4/5
	sail::Binop* ins = static_cast<sail::Binop*>(this->ins);
	sail::Variable* lhs = ins->get_lhs();

	sail::Symbol* op1 = ins->get_first_operand();
	sail::Symbol* op2 = ins->get_second_operand();

	//AccessPath* lhs_ap = AccessPath::get_ap_from_symbol(lhs);
	AccessPath* lhs_ap = get_ap_from_symbol(lhs, mp);

	//AccessPath* rhs_op1 = AccessPath::get_ap_from_symbol(op1);
	AccessPath* rhs_op1 = get_ap_from_symbol(op1, mp);
	symbol_ap.push_back(build_symbol_ap_mem(lhs, lhs_ap));
	symbol_ap.push_back(build_symbol_ap_mem(op1, rhs_op1));
	if (op1->get_type()->is_pointer_type()) {
		il::pointer_type* p_t = static_cast<il::pointer_type*>(op1->get_type());
		if (!(p_t->get_inner_type()->is_record_type())) { //not record type, then it should be array type
			//assert(op2->is_temp());
			assert(op2->get_type()->is_base_type());
			return;
		}
	}

	//AccessPath* rhs_op2 = AccessPath::get_ap_from_symbol(op2);
	AccessPath* rhs_op2 = get_ap_from_symbol(op2, mp);
	symbol_ap.push_back(build_symbol_ap_mem(op2, rhs_op2));

}
void InsMemAnalysis::get_load_aps(vector<symbol_ap_mem*>& symbol_ap) {
	sail::Load* ins = static_cast<sail::Load*>(this->ins);
	sail::Variable* lhs = ins->get_lhs();
	sail::Symbol* rhs = ins->get_rhs();

	//AccessPath* rhs_ap = AccessPath::get_ap_from_symbol(rhs);
	//AccessPath* lhs_ap = AccessPath::get_ap_from_symbol(lhs);

	AccessPath* rhs_ap = get_ap_from_symbol(rhs, mp);
	AccessPath* lhs_ap = get_ap_from_symbol(lhs, mp);

	symbol_ap.push_back(build_symbol_ap_mem(lhs, lhs_ap));
	symbol_ap.push_back(build_symbol_ap_mem(rhs, rhs_ap));
}

void InsMemAnalysis::get_store_aps(vector<symbol_ap_mem*>& symbol_ap) {
	sail::Store* ins = static_cast<sail::Store*>(this->ins);
	sail::Symbol* lhs = ins->get_lhs();
	sail::Symbol* rhs = ins->get_rhs();

	//AccessPath* rhs_ap = AccessPath::get_ap_from_symbol(rhs);
	//AccessPath* lhs_ap = AccessPath::get_ap_from_symbol(lhs);

	AccessPath* rhs_ap = get_ap_from_symbol(rhs, mp);
	AccessPath* lhs_ap = get_ap_from_symbol(lhs, mp);

	symbol_ap.push_back(build_symbol_ap_mem(lhs, lhs_ap));
	symbol_ap.push_back(build_symbol_ap_mem(rhs, rhs_ap));

}




void InsMemAnalysis::get_address_string_aps(vector<symbol_ap_mem*>& symbol_ap) {
	sail::AddressString* ins = static_cast<sail::AddressString*>(this->ins);
	sail::Variable* var = ins->get_lhs();
//	il::type* t = var->get_type();
//
//
//	//assert(t->is_pointer_type());

	//AccessPath* var_ap = AccessPath::get_ap_from_symbol(var);
	AccessPath* var_ap = get_ap_from_symbol(var, mp);
	symbol_ap.push_back(build_symbol_ap_mem(var, var_ap));

	string s = ins->get_string();
	StringLiteral* str_lit = StringLiteral::make(s);
	symbol_ap.push_back(build_symbol_ap_mem(NULL, str_lit));
}




void InsMemAnalysis::get_cast_aps(vector<symbol_ap_mem*>& symbol_ap) {
	sail::Cast* ins = static_cast<sail::Cast*>(this->ins);

	sail::Variable* lhs_var = ins->get_lhs();
	sail::Symbol* rhs_var = ins->get_rhs();

//	AccessPath* lhs_ap = AccessPath::get_ap_from_symbol(lhs_var);
//	AccessPath* rhs_ap = AccessPath::get_ap_from_symbol(rhs_var);
	AccessPath* lhs_ap = get_ap_from_symbol(lhs_var, mp);
	AccessPath* rhs_ap = get_ap_from_symbol(rhs_var, mp);

	symbol_ap.push_back(build_symbol_ap_mem(lhs_var, lhs_ap));
	symbol_ap.push_back(build_symbol_ap_mem(rhs_var, rhs_ap));
}





//3 members
void InsMemAnalysis::get_memory_alloc_aps(vector<symbol_ap_mem*>& symbol_ap) {
	sail::FunctionCall* fc = static_cast<sail::FunctionCall*>(this->ins);
	sail::Variable* lhs_var = fc->get_lhs();

	AccessPath* lhs_ap = get_ap_from_symbol(lhs_var, mp);
	symbol_ap.push_back(build_symbol_ap_mem(lhs_var, lhs_ap));

	int line_number = ins->get_line_number();
	int instruction_num = instruction_number;
	Identifier id = mp->get_function()->get_identifier();
	alloc_info ai;
	mp->get_call_manager()->generate_different_allocsite_id(line_number, instruction_num, id, ai);



	//this one is really possible that being unique out!
	cout << "$$$ CCCCCCCCCalling alloc make() " << endl;
	Alloc* alloc_ap = Alloc::make(ai, lhs_var->get_type(), NULL,
			false);
	cout << "$$$ EEEEEEEEEEEnd of calling alloc make () " << endl;
	symbol_ap.push_back(build_symbol_ap_mem(NULL, alloc_ap));

	sail::Symbol* alloc_size;
	AccessPath* size_ap = NULL;

	//this is the alloc array;
	if (fc->get_arguments()->size() > 0) {
		alloc_size = (*fc->get_arguments())[0];


	} else //alloc_record type
	{
		if (fc->get_lhs() == NULL) //return var, should be impossible here
			alloc_size = new sail::Constant(0, true, 32);
		else {
			il::type* lhs_type = fc->get_lhs()->get_type();
			int size = lhs_type->get_size() / 8;
			alloc_size = new sail::Constant(size, true, 32);
		}
	}

	size_ap = get_ap_from_symbol(alloc_size, mp);
	symbol_ap.push_back(build_symbol_ap_mem(alloc_size, size_ap));

}



symbol_ap_mem* InsMemAnalysis::build_symbol_ap_mem(sail::Symbol* symb,
		AccessPath* ap) {
	symbol_ap_mem* sam = new symbol_ap_mem();
	sam->p = ap;
	sam->s = symb;
	return sam;
}


//void  InsMemAnalysis::get_iterator(
//			vector<sail::Instruction*>::iterator it) {
//	for (unsigned int k = 0; k != this->instruction_number; k++) {
//		it++;
//	}
//}
