#include "ApTree.h"
#define DEBUG false
#define CHECK_AP_RESOLVE false

/******************************
 * ap_node_info struct
 ***************************/

ap_node_info::ap_node_info(Term* t, long int opcode, int l, int index) {
	term = t;
	op_code = opcode;
	level = l;
	idx = index;
}

string ap_node_info::to_string() {
	string str = "Level:  ";
	str += int_to_string(level);
	str += "  ";
	str += "Index:   ";
	str += int_to_string(idx);
	str += "   ";
	str += "Term:   (";
	str += operator_to_string(op_code);
	str += ")";
	str += term->to_string();
	str += "\n";
	return str;
}

/***********************
 * ap_node struct
 ***********************/

ap_node::ap_node(ap_node_info* node_info, ap_node* pred) {
	//resolved = false;
	this->pred = pred;
	this->node_info = node_info;
	succes = new vector<ap_node*>();
}

void ap_node::add_to_succes(ap_node* apn) {
	succes->push_back(apn);
	unresolved_succes_idx.push_back(apn->get_idx());
}

int ap_node::get_idx() {
	return node_info->idx;
}

long int ap_node::get_op_code() {
	return node_info->op_code;
}


string ap_node::to_string() {
	string str;
	str += "PRED:  == {";
	if (pred != NULL) {
		str += int_to_string(pred->get_idx());
	}
	str += "}";
	str += "\n";
	str += node_info->to_string();
	//str += "\n";
	str += "SUCCES ::  ";
	int i = 0;

	for (vector<ap_node*>::iterator it = succes->begin(); it != succes->end();
			it++) {
		if (i == 0)
			str += "\n";
		str += "[";
		str += int_to_string(i++);
		str += "]";
		str += (*it)->node_info->to_string();
	}
	return str;
}

/***********************
 * ApTree class
 ***********************/


bool ApTree::operator < (const ApTree& other) const{
	return (target_ap < other.target_ap);
}



string ApTree::to_string() {
	string str;
	map<unsigned int, ap_node*>::iterator it = ap_nodes.begin();
	for (; it != ap_nodes.end(); it++) {
		str += "IDX:: ";
		str += int_to_string(it->first);
		str += "  <<<<<<<<<<<<<<<<<<< ";
		str += "\n";
		str += "NODE CONETENT:: ";
		str += it->second->to_string();
		str += "  >>>>>>>>>>>>>>>>>>>>>";
		str += "\n";
		str += "\n";
	}
	return str;
}





ap_node* ApTree::get_ap_node(Term* t, long int opcode, int l, int index,
		ap_node* pred) {
	ap_node_info* ap_node_if = new ap_node_info(t, opcode, l, index);
	ap_node* apn = new ap_node(ap_node_if, pred);
	return apn;
}


void ApTree::add_to_predecessor(ap_node* ap_node, int pred_index) {
	struct ap_node* p = NULL;
	assert(ap_node != NULL);
	assert(ap_nodes.count(pred_index) > 0);
	p = ap_nodes[pred_index];
	p->add_to_succes(ap_node);
}


bool ApTree::has_unresolved_ap_node(ap_node* ap_node) {
	vector<struct ap_node*>::iterator it_succes = ap_node->succes->begin();
	for (; it_succes != ap_node->succes->end(); it_succes++) {
		struct ap_node* apn = *it_succes;
		unsigned int idx = apn->get_idx();
		if (unresolved_ap_idx.count(idx) > 0)
			return true;
	}
	return false;
}

//based on if this is inside the map
sail::Symbol* ApTree::get_var(Term* t) {
	sail::Symbol* s = get_variable(t);
	return s;
}

sail::Variable* ApTree::already_built_in_solved_ap_symbols(sail::Symbol* rhs_1,
		sail::Symbol* rhs_2, il::binop_type bt) {

	AccessPath* rhs1_ap = get_ap_from_symbol(rhs_1, su->get_mtd_insts_processor());
	AccessPath* rhs2_ap = get_ap_from_symbol(rhs_2, su->get_mtd_insts_processor());

	//AccessPath* rhs1_ap = AccessPath::get_ap_from_symbol(rhs_1);
	//AccessPath* rhs2_ap = AccessPath::get_ap_from_symbol(rhs_2);
	AccessPath* combined_ap = AccessPath::make_arithmetic_ap(rhs1_ap, rhs2_ap,
			bt);

	bool flag = su->get_solved_ap_symbols().count(combined_ap);

	if(flag)
		return ((sail::Variable*)((su->get_solved_ap_symbols())[combined_ap]));


	return NULL;

}


//this one should belongs to return variable, return varible is a Variable ap;
void ApTree::produce_store_inst(sail::Variable*& lhs, sail::Symbol* rhs_1,
		sail::Symbol* rhs_2, il::binop_type bt,
		bool target_of_argument) {

	sail::Variable* sail_v = already_built_in_solved_ap_symbols(rhs_1, rhs_2, bt);

	if (sail_v == NULL) {

		lhs = SummaryUnit::get_summary_unit_var();
		sail::Binop* binop = new sail::Binop(lhs, rhs_1, rhs_2, bt, -1);

		su->add_to_arithmetic_inst(binop);


	} else {
		lhs = sail_v;
	}

	//build assignment
	if (target_of_argument) { //generate another store instruction
		FieldSelection* fs = (FieldSelection*) arg_ap;
		assert(lhs != NULL);
		assert(fs != NULL);
		sail::Store* s = new sail::Store(this->sail_var, lhs,
				fs->get_field_offset(), fs->get_field_name(), -1);

		su->add_to_arithmetic_inst(s);

		su->update_track_include(s, this->sail_var->is_return_variable());
	}

}

void ApTree::produce_arith_inst(sail::Variable*& lhs, sail::Symbol* rhs_1,
		sail::Symbol* rhs_2, bool binop_f, long int op_code,
		bool target_of_argument) {

	if (binop_f) {
		il::binop_type bt;
		get_binop_code(op_code, bt);

		if (store_ins) {
			produce_store_inst(lhs, rhs_1, rhs_2, bt, target_of_argument);

		} else {

			bool need_build_binop_inst = true;

			if (sail_var->is_return_variable()) {

				//check if the binop instruction has already been built
				sail::Variable* sail_var = already_built_in_solved_ap_symbols(
						rhs_1, rhs_2, bt);

				if (sail_var == NULL) {

					lhs = SummaryUnit::get_summary_unit_var();

				} else {

					need_build_binop_inst = false;

					lhs = sail_var;
				}

			} else {

				if (!target_of_argument)
					lhs = SummaryUnit::get_summary_unit_var();
				else
					lhs = this->sail_var;

				//processing invariant
				if ((lhs == rhs_1) || (lhs == rhs_2)) {
					sail::Variable* tem_var =
							SummaryUnit::get_summary_unit_var();
					sail::Assignment* assi = new sail::Assignment(tem_var, lhs);
					su->add_to_arithmetic_inst(assi);


					su->update_track_include(assi, false);


					if (lhs == rhs_1)
						rhs_1 = tem_var;
					else
						rhs_2 = tem_var;
				}
			}

			if (need_build_binop_inst) {
				sail::Binop* binop = new sail::Binop(lhs, rhs_1, rhs_2, bt, -1);
				cout << "generated binop  " << binop->to_string() << endl;
				//su->get_arithmetic_insts().push_back(binop);
				//arithmetic_ins.push_back(binop);
				su->add_to_arithmetic_inst(binop);
			}

			if ((sail_var->is_return_variable()) && (target_of_argument)) {
				sail::Assignment* assi = new sail::Assignment(this->sail_var,
						lhs);
				//this->su->add_to_return_inst(assi);
				su->add_to_return_insts(assi);

				su->update_track_include(assi, true);
			}

		}
	}else{
		cout << "UNOP type " << endl;
		assert(false);
	}
}




void ApTree::traverse_ap_tree() {

	if (CHECK_AP_RESOLVE)
		cout << "unresolved_ap_idx size is :: " << unresolved_ap_idx.size()
				<< endl;


	//terminate condition
	if (unresolved_ap_idx.size() == 0)
		return;



	cout << "**** unresolved_ap size " << unresolved_ap_idx.size() << endl;

	set<unsigned int>::iterator it_idx = unresolved_ap_idx.begin();
	//used to update unresolved set
	set<unsigned int> resolved;

	for (; it_idx != unresolved_ap_idx.end(); it_idx++) {
		//cout << "inside here ! " << endl;
		unsigned int idx = *it_idx;
		assert(ap_nodes.count(idx) > 0);
		ap_node* apn = ap_nodes[idx];

		//if terminals, solve it directly;
		Term* t = apn->node_info->term;

		//it is possible that termial is (e.g. vi->a) a field-selection ap
		if (is_terminal_term(t)) {
			AccessPath* ap = AccessPath::to_ap(t);
			FieldSelection* fs = NULL;

			if(ap->get_ap_type() == AP_FIELD){

				sail::Variable* lhs = NULL;

				fs = (FieldSelection*)ap;

				if (!su->get_solved_ap_symbols().count(ap)) { //generate lhs

					lhs = SummaryUnit::get_summary_unit_var();


					sail::Variable* var = ((Variable*)( ((Deref*)fs->get_inner())->get_inner() ))->get_original_var();

					sail::Load* l = new sail::Load(lhs, var, fs->get_field_offset(), fs->get_field_name(), -1);

					//generate an Load instruction for it

					su->add_to_arithmetic_inst(l);

					(su->get_solved_ap_symbols())[fs] = lhs;

					if (0) {
						cout << endl << endl;
						cout << "HH++ inserting " << fs->to_string()
								<< "   as " << lhs->to_string() << endl;
					}



				}else{ //found, test it

					//we based on lhs is not NULL to gradually build its parents;
					lhs = (sail::Variable*)((su->get_solved_ap_symbols())[fs]);
					//cout << t->to_string() << "has representative " << lhs << endl;

				}
				assert(lhs != NULL);
				idx_sail_symbols[idx] = lhs;

			}else {

				idx_sail_symbols[idx] = get_var(t);
			}


			cout << "**** resolving node === === " << idx << "  "
					<< t->to_string() << endl;
			resolved.insert(idx);
			continue;
		}



		//pick one that all child are resolved
		if (!has_unresolved_ap_node(apn)) { //solve it
			if (CHECK_AP_RESOLVE)
				cout << "Satisfying to resolve " << idx << endl;

			//find all resolved childs;
			sail::Symbol* last_symbol = NULL;
			AccessPath* last_ap = NULL;
			AccessPath* cur_ap = NULL;
			int i = 0;
			for (vector<ap_node*>::iterator it_succes = apn->succes->begin();
					it_succes != apn->succes->end(); it_succes++) {

				ap_node* succ_ap_node = *it_succes;
				Term* term = succ_ap_node->node_info->term;
				unsigned int succ_idx = succ_ap_node->get_idx();
				if (DEBUG)
				//if (1)
					cout << "successor TTTerm is " << term->to_string() << endl;

				//all child symbols are kept in idx_sail_symbols(keep unit is ap_node)
				assert(idx_sail_symbols.count(succ_idx) > 0);

				sail::Symbol* cur_symbol = idx_sail_symbols[succ_idx];

				if (i != 0) { // not the first one
					cur_ap = AccessPath::to_ap(term);
					long int op_code = succ_ap_node->get_op_code();

					sail::Variable* lhs = NULL;



					assert(op_code != INVALID_OPCODE);

					assert(last_ap != NULL);
					assert(cur_ap != NULL);
					if (DEBUG) {
						cout << "first ap " << last_ap->to_string() << endl;
						cout << "second ap is " << cur_ap->to_string() << endl;
						cout << "opcode is " << operator_to_string(op_code)
								<< endl;
					}

					AccessPath* generated_ap =
							AccessPath::make_access_path_from_binop(last_ap,
									cur_ap, get_binop_type(op_code));
					assert(generated_ap != NULL);

					//determine of it is the real target of the arg_var/ret_var;
					bool flag =
							this->su->inside_argument_target_in_memory_graph(
									generated_ap, this->arg_ap);


					cout << "generated ap is " << generated_ap->to_string() << endl;
					if (!su->get_solved_ap_symbols().count(generated_ap)) { //generate lhs

						produce_arith_inst(lhs, last_symbol, cur_symbol, true,
								op_code, flag);
						cout << "** generated_ap " << generated_ap->to_string() << endl;
						cout << "** lhs is " << lhs->to_string() << endl;
						(su->get_solved_ap_symbols())[generated_ap] = lhs;

						if (DEBUG) {
							cout << endl << endl;
							cout << "HH++ inserting "
									<< generated_ap->to_string() << "   as "
									<< lhs->to_string() << endl;
						}

					} else { //find lhs

						lhs =
							(sail::Variable*) ((su->get_solved_ap_symbols())[generated_ap]);
						//cout << " found in solved_ap_symbol " << "representative  is variable " << lhs->to_string() << endl;
					}
					assert(lhs != NULL);
					last_symbol = lhs;
					last_ap = generated_ap;
				} else { //first one

					long int op_code = succ_ap_node->get_op_code();
					last_symbol = cur_symbol;
					/////////////
					if (op_code == -2) { // "-"
						assert(term != NULL);
						AccessPath* cur_ap = AccessPath::to_ap(term);
						ConstantValue* c = new ConstantValue(-1);
						sail::Constant* sailc = new sail::Constant(-1, true,
								32);
						sail::Variable* lhs = NULL;
						//13 means multiply
						AccessPath* generated_ap =
								AccessPath::make_access_path_from_binop(cur_ap,
										c, il::_MULTIPLY);
						bool flag =
								this->su->inside_argument_target_in_memory_graph(
										generated_ap, this->arg_ap);
						//if (!solved_ap_symbols.count(generated_ap)) {
						if (su->get_solved_ap_symbols().count(generated_ap)) {
							produce_arith_inst(lhs, sailc, cur_symbol, true, 13,
									flag);
							//solved_ap_symbols[generated_ap] = lhs;
							(su->get_solved_ap_symbols())[generated_ap] = lhs;

							if (0) {
								cout << endl << endl;
								cout << "HH++ inserting "
										<< generated_ap->to_string() << "   as "
										<< lhs->to_string() << endl;
							}
						} else {

							lhs =
									(sail::Variable*) ((su->get_solved_ap_symbols())[generated_ap]);
						}
						last_ap = generated_ap;
						last_symbol = lhs;
					} else { // "" or "+"

						last_ap = AccessPath::to_ap(term);

					}

				}

				i++;
			}

			//update idx_sail_symbol;
			if (last_symbol != NULL) {
				idx_sail_symbols[idx] = last_symbol;
				resolved.insert(idx);
			}
		}
	}


	//update unresolved set
	set<unsigned int>::iterator resolved_idx_it = resolved.begin();
	for (; resolved_idx_it != resolved.end(); resolved_idx_it++) {
		unsigned int resolved_idx = *resolved_idx_it;
		assert(unresolved_ap_idx.count(resolved_idx) >0);
		if (CHECK_AP_RESOLVE)
			cout << "resolved IDX --- " << resolved_idx << endl;
		unresolved_ap_idx.erase(resolved_idx);
	}

	traverse_ap_tree();

}


ap_node* ApTree::build_arithvalue_ap_expression_tree(Term* t, int level,
		long int op_code, ap_node* pred) {

	level++;
	assert(t != NULL);
	switch (t->get_term_type()) {
	case CONSTANT_TERM:
	case VARIABLE_TERM: {
		//cout << " === op_code is " << op_code << endl;
		ap_node* apn = get_ap_node(t, op_code, level, this->ap_node_counter++,
				pred);
		add_to_ap_graph(apn);
		return apn;
	}
		break;
	case FUNCTION_TERM: {
		FunctionValue* fv = (FunctionValue*) t;
		FunctionTerm* fn = fv;

		if(DEBUG){
		indent_print(level);
		cout << "|| function_term >>> " << fn->to_string() << " ("
				<< operator_to_string(fv->get_fun_type()) << ")" << endl;
		}
		long int cur_op_code = fv->get_fun_type();
		cout << "function_term type >>>>>>>> ? " << cur_op_code << endl;
		cout << "*** ap type is "<< AccessPath::to_ap(fn)->get_ap_type() << endl;

		//generate a node, and put it into graph

		AccessPath* ap = AccessPath::to_ap(fn);

		ap_node* apn = get_ap_node(fn, op_code, level, this->ap_node_counter++,
				pred);
		add_to_ap_graph(apn);

		//some how this is also a terminal term
		if(ap->get_ap_type() == AP_FIELD){
			//cout << " == op_code is " << op_code << endl;
			return apn;
		}

		//end if this is a terminal term
		int parent_idx = apn->get_idx();

		vector<Term*> terms = fn->get_args();
	//	assert(terms.size() == 2);
		for (vector<Term*>::iterator it = terms.begin(); it != terms.end();
				it++) {
			if(DEBUG){
				indent_print(level);
				cout << "<< sub_term == " << (*it)->to_string() << endl;
			}
			//add it to its parent;
			ap_node* cur_ap_node = NULL;
			if (it == terms.begin()){
				cur_ap_node = build_arithvalue_ap_expression_tree(*it, level,
						INVALID_OPCODE, apn);
			}
			else{
				cur_ap_node = build_arithvalue_ap_expression_tree(*it, level,
						cur_op_code, apn);
			}

			if(cur_ap_node != NULL)
				add_to_predecessor(cur_ap_node, parent_idx);
			if(cur_ap_node == NULL)
				return NULL;
		}
		return apn;
		break;
	}
	case ARITHMETIC_TERM: { //100a1, only contain one element, default should be "+"
		//need to self define type of arithmetic_term;

		ArithmeticTerm* at = (ArithmeticTerm*) t;
		if (DEBUG) {
			indent_print(level);
			cout << "|| arithmetic term >>> " << at->to_string() << endl;
		}

		ap_node* apn = get_ap_node(at, op_code, level, this->ap_node_counter++,
				pred);
		add_to_ap_graph(apn);
		int parent_idx = apn->get_idx();



		//first decide the combination order;
		//cout << "************** %%%%%%%%%%%%%%%%%%%%% ************** " << endl;
		vector<term> ts = at->get_vector_elems();

		//cout << "######## t size is " << ts.size() << endl;
		if (ts.size() == 1) {
			ap_node* cur_ap_node = NULL;
			Term* const_t = ConstantTerm::make((*ts.begin()).coef);
			cur_ap_node = build_arithvalue_ap_expression_tree(const_t, level,
					INVALID_OPCODE, apn);
			add_to_predecessor(cur_ap_node, parent_idx);



			Term* t = (*ts.begin()).t;
			cur_ap_node = build_arithvalue_ap_expression_tree(t, level, 13,
					apn);
			add_to_predecessor(cur_ap_node, parent_idx);

		}
		else {
			for (vector<term>::iterator it_1 = ts.begin(); it_1 != ts.end();
					it_1++) {

				ap_node* cur_ap_node = NULL;
				if (it_1 == ts.begin()) {
					if(0)
						cout << "* * * sub_term is " << (*it_1).t->to_string()
							<< endl;
					cur_ap_node = build_arithvalue_ap_expression_tree((*it_1).t,
							level, INVALID_OPCODE, apn);
				} else {
					//could always be plus operator
					long int c = -1;
					if(0)
						cout << "* * * sub_term is " << (*it_1).t->to_string()
							<< endl;

					cur_ap_node = build_arithvalue_ap_expression_tree((*it_1).t,
							level, c, apn);
				}
				add_to_predecessor(cur_ap_node, parent_idx);
			}
	}
		return apn;
		break;
	}
	default:
		assert(false);

	}
}


void ApTree::produce_arith_fun_insts(AccessPath* cur_ap) {



	Term* t = NULL;
	if (cur_ap->get_ap_type() == AP_ARITHMETIC) {
		ArithmeticValue* av = static_cast<ArithmeticValue*>(cur_ap);
		t = av;
	} else {
		FunctionValue* fv = static_cast<FunctionValue*>(cur_ap);
		FunctionTerm* ft = fv;
		t = ft;
	}

	//cout << " --------------------************|||||||||||||||   |||||||||||||||**********-------------------- " << endl;


	//cout << "Term is " << t->to_string() << endl;
	if(t->to_string().find("UNCERTAINTY") != string::npos)
		return;


	build_arithvalue_ap_expression_tree(t, 0, INVALID_OPCODE, NULL);

	if(DEBUG){
	//if (1){
		string str = to_string();
		cout << "TESTING " << endl << str << endl;
	}

	traverse_ap_tree();

}

