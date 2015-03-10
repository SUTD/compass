/*
 * ComputePO.cpp
 *
 *  Created on: May 22, 2012
 *      Author: boyang
 */

#include "ComputePO.h"
#include "POE.h"
#include "Abductor.h"
#include "util.h"
#include "il/binop_expression.h"
#include "ModSet.h"



#define DEBUG false
#define UNKNOWN_STR "unknown"
#define OBSERVED "observed"
#define NUM_UNROLL 3
#define UNROLLING false
#define USING_MODSET true


ComputePO::ComputePO(POEKey *poekey_entry, Function *func, vector<Function *>& func_vector) {

	/*
	 * get the function's PO structure and add it to the original POE map
	 */
	Block *b_entry = poekey_entry->getBlock();
	BasicBlock *bb_entry;
	this->entry_func = func;

	if(poekey_entry->is_basicblock())
	{
		bb_entry = static_cast<sail::BasicBlock *> (b_entry);
	}
	else
	{
		//entry block should always be basic block
		assert(false);
	}

	CallingContext *calling_seq = poekey_entry->getcallingcontext();
	this->func_vector = func_vector;
	map<Block*, ProofObligation*> cur_map;
	get_fuc_po(bb_entry, cur_map);

	map<Block *, ProofObligation *>::iterator mapBlock_iter = cur_map.begin();
	for(; mapBlock_iter != cur_map.end(); mapBlock_iter++)
	{
		Block *b = mapBlock_iter->first;
		ProofObligation *po = mapBlock_iter->second;
		POEKey *key = new POEKey(b, calling_seq);
		map_poekey_po.insert(pair<POEKey *, ProofObligation * >(key, po));
	}

	vector<Instruction*> instructions;
	get_follow_instructions(bb_entry, instructions);
	vector<Instruction*> ::iterator iter;
	for (iter = instructions.begin(); iter != instructions.end(); ++iter) {
		if((*iter)->get_instruction_id() == FUNCTION_CALL)
		{
			FunctionCall* functionCall = static_cast<sail::FunctionCall*> (*iter);

			//cout << "line  : "   <<  functionCall->line << endl;
			//vector<Symbol*>* syms = functionCall->get_arguments();

			//--------------------maybe can be used later ----------------------
			//			vector<Term*> args ;
			//			for(iter = (*syms).begin(); iter != (*syms).end(); iter++)
			//			{
			//				Term * term = NULL;
			//				if ((*iter)->is_variable()) {
			//					Variable* v = static_cast<Variable *> (*iter);
			//					term = VariableTerm::make(v->get_var_name());
			//				} else {
			//					Constant* c = static_cast<Constant *> (*iter);
			//					term = ConstantTerm::make(c->get_integer());
			//				}
			//				args.push_back(term);
			//			}
			//----------------------------------------end ----------------------



			string name = functionCall->get_function_name();
			size_t found;
			found = name.find(UNKNOWN_STR);
			if (found!=string::npos)
			{
				continue;
			}

			found = name.find(OBSERVED);
			if (found!=string::npos)
			{
				continue;
			}

			Function *cur_func = function_finder(name);
			/*
			 * update calling seq and call poe_map_adder
			 */
			CallingContext *new_calling_seq = new CallingContext(*calling_seq);
			new_calling_seq->add_context(func,  functionCall->line);
			poe_map_adder(cur_func, new_calling_seq);
		}
	}


}


ComputePO::ComputePO(Function* callingfunc, map<POEKey*,
		ProofObligation*>& map_poekey_po, vector<Function *>& func_vector)
{
	this->entry_func = callingfunc;
	this->map_poekey_po = map_poekey_po;
	this->func_vector = func_vector;
}


void ComputePO::poe_map_adder(Function * func, CallingContext * calling_seq)
{
	/*
	 * get the function's PO structure and add it to the original POE map
	 */
	map<Block*, ProofObligation*> cur_map;
	get_fuc_po(func->get_entry_block(), cur_map);
	map<Block *, ProofObligation *>::iterator mapBlock_iter;
	for(mapBlock_iter = cur_map.begin(); mapBlock_iter != cur_map.end(); mapBlock_iter++)
	{
		Block *b = mapBlock_iter->first;
		ProofObligation *po = mapBlock_iter->second;
		POEKey *key = new POEKey(b, calling_seq);
		map_poekey_po.insert(pair<POEKey *, ProofObligation * >(key, po));
	}

	/*
	 * recursively find structures of function call
	 */
	vector<Instruction*> *instructions = func->get_body();
	vector<Instruction*> ::iterator iter;
	for (iter = (*instructions).begin(); iter != (*instructions).end(); ++iter) {
		if((*iter)->get_instruction_id() == FUNCTION_CALL)
		{
			FunctionCall* functionCall = static_cast<sail::FunctionCall*> (*iter);

			//--------------------maybe can be used later -------------------------
			//			vector<Symbol*>* syms = functionCall->get_arguments();
			//		  	vector<Symbol*>::iterator iter;
			//			vector<Term*> args ;
			//			for(iter = (*syms).begin(); iter != (*syms).end(); iter++)
			//			{
			//				Term * term = NULL;
			//				if ((*iter)->is_variable()) {
			//					Variable* v = static_cast<Variable *> (*iter);
			//					term = VariableTerm::make(v->get_var_name());
			//				} else {
			//					Constant* c = static_cast<Constant *> (*iter);
			//					term = ConstantTerm::make(c->get_integer());
			//				}
			//				args.push_back(term);
			//			}
			//----------------------------------------------------------------------

			string name = functionCall->get_function_name();
			size_t found;
			found = name.find(UNKNOWN_STR);
			if (found!=string::npos)
			{
				continue;
			}
			found = name.find(OBSERVED);
			if (found!=string::npos)
			{
				continue;
			}

			Function *cur_func = function_finder(name);//, args);
			/*
			 * update calling seq and call poe_map_adder
			 */
			CallingContext *new_calling_seq = new CallingContext(*calling_seq);
			new_calling_seq->add_context(func,  functionCall->line);
			poe_map_adder(cur_func, new_calling_seq);
		}
	}
}



Function * ComputePO::function_finder(string name)//, vector<Term*> args)
{
	/*
	 * only compare function name and number of args
	 */
	vector<Function *>::iterator iter;
	for(iter = func_vector.begin(); iter != func_vector.end(); iter++)
	{
		function_declaration* declare = (*iter)->get_original_declaration();
		string name_declare =  declare->get_name();
		vector<variable_declaration*> args_declare = declare->get_arguments();
		if(std::strcmp(name_declare.c_str(), name.c_str()) == 0)// && args.size() == args_declare.size())
		{
			return (*iter);
		}
	}
	cout << "did not find function " << name << endl;
	assert(false);
}



void ComputePO::get_fuc_po(BasicBlock * b_entry, map<Block*, ProofObligation*>& cur_map) {

	//generate map
	set<sail::Block*, sail::CompareBlock> worklist;
	worklist.insert(b_entry);
	ProofObligation * po_cur;

	while(worklist.size() > 0){
		sail::Block* b_cur = *worklist.begin();
		worklist.erase(b_cur);

		if(b_cur->is_basicblock()){
			po_cur = new ProofObligation();
		}else{
			po_cur= new ProofObligation();
			SuperBlock* spb_cur = static_cast<SuperBlock *> (b_cur);
			BasicBlock* bb_sb_entry = spb_cur->get_entry_block();

			//Add entry of nested while loop into the worklist
			worklist.insert(bb_sb_entry);

		}
		cur_map.insert(pair<Block *, ProofObligation *>(b_cur, po_cur));
		set<CfgEdge*>& set_Edges = b_cur->get_successors();
		set <CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for (; iter_Edges!=set_Edges.end(); iter_Edges++)
		{
			worklist.insert((*iter_Edges)->get_target());
		}
	}
}


void ComputePO::get_follow_instructions(BasicBlock* entry, vector<Instruction*> & vect_inst)
{
	set<sail::Block*, sail::CompareBlock> worklist;
	worklist.insert(entry);

	while(worklist.size() > 0){
		sail::Block* b_cur = *worklist.begin();
		worklist.erase(b_cur);

		if(b_cur->is_basicblock()){
			BasicBlock *bb = static_cast<BasicBlock *> (b_cur);
			vector<Instruction*> cur_statements = bb->get_statements();
			vect_inst.insert(vect_inst.end(), cur_statements.begin(), cur_statements.end());
		}else{
			SuperBlock* spb_cur = static_cast<SuperBlock *> (b_cur);
			BasicBlock* bb_sb_entry = spb_cur->get_entry_block();
			//Add entry of nested while loop into the worklist
			worklist.insert(bb_sb_entry);
		}

		set<CfgEdge*>& set_Edges = b_cur->get_successors();
		set <CfgEdge*>::iterator iter_Edges;
		for (iter_Edges=set_Edges.begin(); iter_Edges!=set_Edges.end(); iter_Edges++)
		{
			worklist.insert((*iter_Edges)->get_target());
		}
	}
}



ComputePO::ComputePO(Function * entry,  vector<Function *>& func_vector)
{
	this->entry_func = entry;
	this->func_vector = func_vector;
}


ComputePO::~ComputePO() {
	// TODO Auto-generated destructor stub
}


void ComputePO::sandwitch_loop_sp(POEKey* pk_sb, Constraint phi)
{
	Block* b = pk_sb->getBlock();
	SuperBlock* spb = NULL;
	if(b->is_superblock())
	{
		spb = static_cast<SuperBlock *> (b);

	}else
	{
		assert(false);
	}

	BasicBlock* b_exit = spb->get_exit_block();
	POEKey* pk_exit = new POEKey(b_exit, pk_sb->getcallingcontext());

	/*
	 * find loop body entry
	 */
	set<CfgEdge*> edges = spb->get_entry_block()->get_successors();
	set<CfgEdge*>::iterator edge_iter;
	Block * body_entry = NULL;
	for(edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter++)
	{
		body_entry = (*edge_iter)->get_target();
		if(body_entry->get_block_id() == spb->get_exit_block()->get_block_id())
		{
			continue;
		}
		else
		{
			break;
		}
	}
	POEKey* pk_body_entry = new POEKey(body_entry, pk_sb->getcallingcontext());
	//put the body entry block in the queue
	ProofObligation * po_cur = find_ProofObligation(pk_body_entry);
	po_cur->add_constraint(phi, true);
	worklist.insert(pk_body_entry);

	// true means forward
	// the second arg is not very useful in here
	compute_block_swp(true, pk_exit, false);
}




void ComputePO::sandwitch_loop_wp(POEKey* pk_sb, Constraint cons)
{
	Block* b = pk_sb->getBlock();
	SuperBlock* sb = NULL;
	if(b->is_superblock())
	{
		sb = static_cast<SuperBlock *> (b);
	}else
	{
		assert(false);
	}

	set <Block*> set_blocks = sb->get_body();
	set <Block*>::iterator set_blocks_iter = set_blocks.begin();
	for(; set_blocks_iter != set_blocks.end(); set_blocks_iter++)
	{
		if((*set_blocks_iter)->is_basicblock())
		{
			BasicBlock* bb = static_cast<BasicBlock *>(*set_blocks_iter);
			//ignore empty blocks
			if(bb->get_statements().size() <= 0)
			{
				continue;
			}
			Instruction * inst = bb->get_statements().back();
			if(inst->get_instruction_id() == LOOP_INVOCATION)
			{
				POEKey* pk_bb = new POEKey(bb, pk_sb->getcallingcontext());
				ProofObligation * po_cur = find_ProofObligation(pk_bb);
				if(po_cur!= NULL){
					po_cur->add_constraint(cons, false);
					//q_block.push(bb);
					worklist.insert(pk_bb);
				}else
				{
					assert(false);
				}
			}

		}
	}

	//put exit block in the queue
	BasicBlock* b_exit = sb->get_exit_block();
	POEKey* pk_exit = new POEKey(b_exit, pk_sb->getcallingcontext());
	ProofObligation * po_exit = find_ProofObligation(pk_exit);
	po_exit->add_constraint(cons, false);
	worklist.insert(pk_exit);


	set<CfgEdge*> edges = sb->get_entry_block()->get_successors();
	set<CfgEdge*>::iterator edge_iter;
	Block * body_entry = NULL;
	for(edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter++)
	{
		body_entry = (*edge_iter)->get_target();
		if(body_entry->get_block_id() == sb->get_exit_block()->get_block_id())
		{
			continue;
		}
		else
		{
			break;
		}
	}

	POEKey* pk_body_entry = new POEKey(body_entry, pk_sb->getcallingcontext());

	// true means forward
	//the second argument is for extra_q_info, which is not useful in here
	compute_block_swp(false, pk_body_entry, false);
}


void ComputePO::updateInnerPO(map<POEKey*, ProofObligation*>& mapBlock, list<POEKey*> list_loop_id)
{
	list<POEKey*>::iterator list_iter;
	for(list_iter = list_loop_id.begin(); list_iter != list_loop_id.end(); list_iter++)
	{
		ProofObligation* new_po = find_ProofObligation(*list_iter);
		map<POEKey*, ProofObligation*>::iterator mapBlock_iter;
		for(mapBlock_iter = mapBlock.begin(); mapBlock_iter != mapBlock.end(); mapBlock_iter++)
		{
			if(*mapBlock_iter->first == *(*list_iter) && mapBlock_iter->first->is_superblock())
			{
				Constraint c_pre = new_po->getPrecondition();
				eliminate_temp(c_pre);
				mapBlock_iter->second->setPrecondition(c_pre&mapBlock_iter->second->getPrecondition());
				Constraint c_post = new_po->getPostcondition();
				eliminate_temp(c_post);
				mapBlock_iter->second->setPostcondition(c_post&mapBlock_iter->second->getPostcondition());
			}
		}
	}
}


Constraint ComputePO::compute_wp_between_blocks(POEKey* pk_entry,
		POEKey* pk_bottom, POEKey* pk_top, Constraint c_start)
{
	//ComputePO cpo(b_entry);
	ComputePO cpo(pk_entry, entry_func, func_vector);
	return cpo.compute_wp_between_blocks_helper(pk_bottom, pk_top,c_start);
}



Constraint ComputePO::compute_wp_between_blocks_helper(POEKey* pk_bottom,
		POEKey* pk_top, Constraint c_start)
{
	/*
	 * put exit block in the queue
	 */
	ProofObligation * po_bottom = find_ProofObligation(pk_bottom);
	po_bottom->add_constraint(c_start, false);
	worklist.insert(pk_bottom);
	compute_block_swp(false, pk_top, false);
	//return extra_q_info;
	return extra_above_info;
}


Constraint ComputePO::compute_loop_condition(POEKey* pk_sb)
{

	SuperBlock* sb = static_cast<SuperBlock*>(pk_sb->getBlock());

	set<CfgEdge*> edges = sb->get_entry_block()->get_successors();
	set<CfgEdge*>::iterator edge_iter;

	Block * body_entry = NULL;
	for(edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter++)
	{
		body_entry = (*edge_iter)->get_target();
		if(body_entry->get_block_id() == sb->get_exit_block()->get_block_id())
		{
			continue;
		}
		else
		{
			break;
		}
	}
	Constraint breakInfo(false);

	BasicBlock* bb_exit = sb->get_exit_block();
	edges = bb_exit->get_predecessors();
	for(edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter++)
	{
		Block* b_cur = (*edge_iter)->get_source();

		if(b_cur->get_block_id() != sb->get_entry_block()->get_block_id())
		{
			Symbol * s = (*edge_iter)->get_cond();
			if(s == NULL)
			{
				//changed_by_boyang 11/28/2012 Hope there is no side-affect.
				//cout <<"s is null" << endl;
				continue;
			}
			Term* new_t = symbol_to_term(s);
			Term* t_c1 = ConstantTerm::make(1);
			Constraint cons = Constraint(new_t, t_c1, ATOM_EQ);
			//while_c = while_c & !cc;
			CallingContext* cc = pk_sb->getcallingcontext();
			POEKey* pk_entry = new POEKey(sb->get_entry_block(), cc);
			POEKey* pk_cur= new POEKey(b_cur, cc);
			POEKey* pk_body = new POEKey(body_entry, cc);
			breakInfo = breakInfo | compute_wp_between_blocks(pk_entry, pk_cur, pk_body, cons);
		}
	}


	/*
	 * compute sp while loop continue condition
	 */
	BasicBlock* bb_sb_entry = sb->get_entry_block();
	vector<Instruction*> statements = bb_sb_entry->get_statements();
	Constraint t(true);

	CallingContext* calling_seq = pk_sb->getcallingcontext();
	if( calling_seq->get_curfunc_name() == " ")
	{
		function_declaration* declare = this->entry_func->get_original_declaration();
		var_prefix = declare->get_name()+"::";
	}else
	{
		this->var_prefix = calling_seq->get_curfunc_name() + "::";
	}

	Constraint while_c  = sp(statements, t);

	for(edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter++)
	{
		Block* b_target = (*edge_iter)->get_target();
		if(b_target->get_block_id() == sb->get_exit_block()->get_block_id())
		{
			Symbol * s = (*edge_iter)->get_cond();
			Constraint new_phi;
			if(s == NULL){
				//changed_by_boyang 11/28/2012
				//assert(false);
			}else{
				Term* new_t = symbol_to_term(s);
				Term* t_c1 = ConstantTerm::make(1);
				Constraint cc = Constraint(new_t, t_c1, ATOM_EQ);
				while_c = while_c & !cc;

			}
		}
	}
	return  breakInfo | while_c;
}


void ComputePO::compute_all_loop_condition()
{
	map<POEKey *, ProofObligation *>::iterator mapPOEKey_iter = map_poekey_po.begin();

	for(; mapPOEKey_iter != map_poekey_po.end(); mapPOEKey_iter++)
	{
		if(mapPOEKey_iter->first->is_superblock())
		{
			//SuperBlock* sb = static_cast<SuperBlock*>(mapPOEKey_iter->first->getBlock());
			Constraint loopCondition = compute_loop_condition(mapPOEKey_iter->first);
			set<Term*> all_terms;
			loopCondition.get_terms(all_terms, false);

			set<Term*>::iterator t_iter;
			for(t_iter = all_terms.begin(); t_iter!= all_terms.end(); t_iter++)
			{
				if((*t_iter)->type == FUNCTION_TERM){
					FunctionTerm* ft = static_cast<FunctionTerm*>(*t_iter);
					string fun_name = ft->get_name();

					size_t found;
					found = fun_name.find(UNKNOWN_STR);
					if (found!=string::npos)
					{
						Term* term_func = (*t_iter);
						Term* new_term = VariableTerm::make("__" + fun_name);
						loopCondition.replace_term(term_func, new_term);
					}
				}
			}

			eliminate_temp(loopCondition);
			//cout << "loop condition !! : " << loopCondition.to_string() << endl;
			mapPOEKey_iter->second->setWhile_condition(loopCondition);
		}
	}
}



ProofObligation * ComputePO::find_ProofObligation(POEKey*key){

	map<POEKey *, ProofObligation *>::iterator mapPOEKey_iter = map_poekey_po.begin();
	for(; mapPOEKey_iter != map_poekey_po.end(); mapPOEKey_iter++)
	{
		if(*mapPOEKey_iter->first == *key){
			return mapPOEKey_iter->second;
		}
	}
	assert(false);
	//return NULL;
}



void ComputePO::compute_block_swp(bool is_forward, POEKey* key_end_block,
		bool nested_analysis)
{
	bool set_prefix = false;
	while(worklist.size() > 0){

		POEKey * key_b_p;
		if(is_forward)
		{
			key_b_p = *worklist.begin();
			worklist.erase(key_b_p);
		}else
		{
			key_b_p = *worklist.rbegin();
			worklist.erase(key_b_p);
		}

		if(!set_prefix)
		{
			CallingContext* calling_seq = key_b_p->getcallingcontext();
			if( calling_seq->get_curfunc_name() == " ")
			{
				function_declaration* declare =
						this->entry_func->get_original_declaration();
				var_prefix = declare->get_name()+"::";
			}else
			{
				this->var_prefix = calling_seq->get_curfunc_name() + "::";
			}
			set_prefix = true;
		}

		CallingContext *cur_callingcontext = key_b_p->getcallingcontext();
		ProofObligation* po_cur = find_ProofObligation(key_b_p);

		if(DEBUG)
		{
			if(is_forward)
			{
				cout << "sp    ";
			}else
			{
				cout << "wp    ";
			}
			cout << "#########################block id: "
					<< key_b_p->get_block_id() <<"#############################"<<endl;
		}

		Constraint phi;
		if(is_forward)
		{
			phi = po_cur->getPrecondition();
		}else
		{
			phi = po_cur->getPostcondition();
		}


		/*
		 * compute current block
		 */
		if(key_b_p->is_basicblock())
		{
			BasicBlock* bb = static_cast<BasicBlock *>(key_b_p->getBlock());
			cur_eval_key = key_b_p;
			//CallingContext* cc = key_b_p->getcallingcontext();
			//if(entry_func!=NULL)
			//{
			//
			//	function_declaration* declare = this->entry_func->get_original_declaration();
			//	var_prefix = declare->get_name()+"::";
			//}
			//else
			//{
			//	var_prefix = " ::"; //space, distinguish with global variables
			//}

			vector<Instruction*> statements = bb->get_statements();

			if(is_forward)
			{
				phi = sp(statements, phi);
			} else
			{
				phi = wp(statements, phi);
			}
		}else{
			/*
			 * I has been computed initially
			 * R has been computed lately by using compute_all_superBlock_R()
			 * add entry block of the SuperBlock in queue
			 */
			SuperBlock* sb_p = static_cast<SuperBlock *> (key_b_p->getBlock());
			if(is_forward)
			{
				//======new added 12/02/12=======
				if(USING_MODSET)
				{
					//if M is the mod set of the loop and P is the computed precondition,
					//update the loop invariant I to "Exists M. P" & I
					const set<VariableTerm*>& mod_set = ms.get_modset(sb_p);
					set<VariableTerm*> mod_set_prefix;
					for(auto itt = mod_set.begin(); itt!= mod_set.end(); itt++) {
						VariableTerm* cur = *itt;
						if(cur->to_string().find("__temp") != string::npos)
							continue;
						Term* t = VariableTerm::make(var_prefix + cur->get_name());
						VariableTerm* vt = static_cast<VariableTerm*>(t);
						//cout << "variable term : " << vt->to_string() << endl;
						mod_set_prefix.insert(vt);
					}
					Constraint p_temp = po_cur->getPrecondition();
					Constraint t_i = po_cur->getKnown_inv();
					p_temp.eliminate_evars(mod_set_prefix);
					po_cur->setKnown_inv(p_temp&t_i);
				}
				//======end new added 12/02/12=======

				Constraint i = po_cur->getKnown_inv();
				//i = po_cur->getKnown_inv();
				Constraint r = po_cur->getInv_to_show();
				phi = i&r;

				/*
				 * copy the precondition
				 */
				//BasicBlock* bb_entry = sb_p->get_entry_block();
				//POEKey * key_entry = new POEKey(bb_entry, cur_callingcontext);

				set<CfgEdge*> edges = sb_p->get_entry_block()->get_successors();
				set<CfgEdge*>::iterator edge_iter;
				Block * body_entry = NULL;
				for(edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter++)
				{
					body_entry = (*edge_iter)->get_target();
					if(body_entry->get_block_id() == sb_p->get_exit_block()->get_block_id())
					{
						continue;
					}
					else
					{
						break;
					}
				}
				POEKey * key_body_entry = new POEKey(body_entry, cur_callingcontext);
				ProofObligation* po_key_body_entry = find_ProofObligation(key_body_entry);
				po_key_body_entry->add_constraint(phi, true);
				worklist.insert(key_body_entry);
			} else
			{
				//Constraint po_cur_q = po_cur->getPostcondition();
				Constraint i = po_cur->getKnown_inv();
				Constraint r = po_cur->getInv_to_show();
				phi = i&r;  //i&!c
				if(nested_analysis)
				{
					superBlock_wp_adder(sb_p, phi, cur_callingcontext);
				}
			}
		}

		if(!is_forward)
		{
			po_cur->setWp_before(phi);
		}else
		{
			po_cur->setSp_after(phi);
		}

		if(key_end_block != NULL && *key_b_p == *key_end_block)
		{
			//for SuperBlock's body
			if(is_forward)
			{
				//extra_p_info = phi;
				extra_after_info = phi;
			}else
			{
				//extra_q_info = phi;
				extra_above_info = phi;
			}
		}

		/*
		 * deal with follow-up blocks
		 */
		set<CfgEdge*> set_Edges = key_b_p->get_successors();
		if(!is_forward){
			set_Edges = key_b_p->get_predecessors();
		}
		set <CfgEdge*>::iterator iter_Edges=set_Edges.begin();
		for (; iter_Edges!=set_Edges.end(); iter_Edges++)
		{

			Block * b_next = NULL;
			if(is_forward)
			{
				b_next =  (*iter_Edges)->get_target();
			}else
			{
				b_next =  (*iter_Edges)->get_source();
			}

			Symbol * s = (*iter_Edges)->get_cond();
			Constraint new_phi;
			if(s == NULL){
				new_phi = phi;
				//eliminate_temp(new_phi);
			}else{
				Term* new_t = symbol_to_term(s);
				Term* t_c1 = ConstantTerm::make(1);
				Constraint cc = Constraint(new_t, t_c1, ATOM_EQ);
				if(is_forward)
				{
					new_phi = phi & cc;
				}else
				{
					new_phi = !cc |phi;
				}
				/*
				 * to eliminate unused variable as soon as possible.
				 * changed 11/30/12
				 */
				eliminate_temp(new_phi, new_t);
			}

			POEKey * key_next = new POEKey(b_next, cur_callingcontext);
			ProofObligation * po_next = find_ProofObligation(key_next);
			//cout << "new_phi :" << endl;
			po_next->add_constraint(new_phi, is_forward);
			worklist.insert(key_next);
		}
	}
}


void ComputePO::superBlock_wp_adder(SuperBlock* sb, Constraint cons,
		CallingContext* cur_callingcontext)
{
	set <Block*> set_blocks = sb->get_body();
	set <Block*>::iterator set_blocks_iter = set_blocks.begin();
	for(; set_blocks_iter != set_blocks.end(); set_blocks_iter++)
	{
		if((*set_blocks_iter)->is_basicblock())
		{
			BasicBlock* bb = static_cast<BasicBlock *>(*set_blocks_iter);
			//ignore the empty block
			if(bb->get_statements().size() <= 0)
			{
				POEKey * key_bb = new POEKey(bb, cur_callingcontext);
				ProofObligation * po_cur = find_ProofObligation(key_bb);
				if(po_cur!= NULL){
					po_cur->add_constraint(cons, false);
					worklist.insert(key_bb);
				}
				continue;
			}
			Instruction * inst = bb->get_statements().back();
			if(inst->get_instruction_id() == LOOP_INVOCATION)
			{
				POEKey * key_bb = new POEKey(bb, cur_callingcontext);
				ProofObligation * po_cur = find_ProofObligation(key_bb);
				if(po_cur!= NULL){
					po_cur->add_constraint(cons, false);
					worklist.insert(key_bb);
				}else
				{
					assert(false);
				}
			}

		}
		//it doesn't matter, if the block is a super block  (nested loop)
	}
}



//for test
void ComputePO::print_out_block(Block * b_p)
{
	if(b_p->is_basicblock())
	{
		BasicBlock* bb = static_cast<BasicBlock *>(b_p);
		vector<Instruction*> statements = bb->get_statements();

		vector<Instruction*>::iterator iter_inst;
		for (iter_inst = statements.begin(); iter_inst != statements.end(); iter_inst++) {
			cout << (*iter_inst)->line << ": " << (*iter_inst)->to_string()<< endl;
		}
	}
}


void ComputePO::print_out_po_info(){
	map<POEKey *, ProofObligation *>::iterator mapPOEKey_iter;
	cout <<endl << "%%%%%%%%%%%%%%%%%%  print_out ComputePO::superpo_info" << endl;
	for(mapPOEKey_iter = map_poekey_po.begin(); mapPOEKey_iter != map_poekey_po.end(); mapPOEKey_iter++)
	{
		if(mapPOEKey_iter->first->is_superblock()){
			cout <<endl << "super block!!" << endl;
			cout << "block INFO"  << mapPOEKey_iter->first->to_string() <<endl;
			cout << "precondition  :"<< mapPOEKey_iter->second->getPrecondition().to_string() << endl;
			cout << "postcondition  :"<< mapPOEKey_iter->second->getPostcondition().to_string() << endl;
			cout << "known_inv (I)  :" << mapPOEKey_iter->second->getKnown_inv().to_string() << endl;
			cout << "inv_to_show (R)  :" << mapPOEKey_iter->second->getInv_to_show().to_string() << endl;
		} else{
			cout <<endl  << "block INFO"  << mapPOEKey_iter->first->to_string()<<endl;
			cout << "precondition  :"<< mapPOEKey_iter->second->getPrecondition().to_string() << endl;
			cout << "postcondition  :"<< mapPOEKey_iter->second->getPostcondition().to_string() << endl;
			cout << "known_inv (I)  :" << mapPOEKey_iter->second->getKnown_inv().to_string() << endl;
			cout << "inv_to_show (R)  :" << mapPOEKey_iter->second->getInv_to_show().to_string() << endl;
		}
	}

}


void ComputePO::get_init_evironment(POEKey *poekey_entry, Constraint begin,
		POEKey *poekey_exit, Constraint end,
		map<POEKey*, ProofObligation*>& map_return){

	int t = clock();
	//call from POE init
	//R
	compute_all_superBlock_R();

	//C   first call this.
	compute_all_loop_condition();

	//Observed & I
	compute_all_observed_and_I();

	// P, Q
	compute_func_sp(poekey_entry, poekey_exit, begin);
	compute_func_wp(poekey_entry, poekey_exit, end);

	//update I based on P
	adjust_I_based_on_P();


	time = clock() - t;
	t = clock();

	if(UNROLLING)
	{
		// Isil's addition
		compute_unrolled_sp();
	}
	unroll_time = clock()-t;

	map_return = this->map_poekey_po;
}


void ComputePO::compute_modsets()
{


	map<POEKey*, ProofObligation*>::iterator it;
	for(it = map_poekey_po.begin(); it != map_poekey_po.end(); it++)
	{
		POEKey* k = it->first;
		Block* b = k->getBlock();
		ms.print_modset(b);

	}
}


void ComputePO::compute_unrolled_sp()
{

	map<POEKey*, ProofObligation*>::iterator it;
	for(it =  map_poekey_po.begin(); it != map_poekey_po.end(); it++)
	{
		POEKey* key = it->first;
		ProofObligation* po = it->second;
		if(!key->is_superblock()) continue;
		SuperBlock* sb = static_cast<SuperBlock*>(key->getBlock());
		Constraint pre = po->getPrecondition();
		eliminate_temp(pre);

		pre = initialize_precond(pre, key);
		cout << "PRECOND: " << pre << endl;
		POEKey* entry = new POEKey(sb->get_entry_block(), key->getcallingcontext());
		POEKey* exit = new POEKey(sb->get_exit_block(), key->getcallingcontext());
		cout << "EXIT BLOCK ID: " << exit->get_block_id() << endl;
		Constraint unroll_sp = pre;

		for(int i=0; i< NUM_UNROLL; i++)
		{
			ComputePO cpo(entry, this->entry_func, this->func_vector);
			cpo.compute_func_sp(entry, exit, pre);
			set<Block*> invocation_blocks;
			sb->get_loop_invocation_blocks(invocation_blocks);
			Constraint sp(false);
			for(auto it = invocation_blocks.begin(); it!= invocation_blocks.end(); it++)
			{
				POEKey* cur = new POEKey(*it, key->getcallingcontext());
				Constraint cur_sp = cpo.get_block_spInfo(cur);
				eliminate_temp(cur_sp);
				sp |= cur_sp;

			}
			sp.sat();
			pre = sp;
			Constraint renamed_sp = add_version_number(sp, key, po, i+1);
			cout << "Renamed sp: " << renamed_sp << endl;
			Constraint new_sp = unroll_sp & renamed_sp;
			if(new_sp.unsat()) break;
			unroll_sp  = new_sp;


		}
		cout <<  "UNROLLED SUPER BLOCK "<< sb->get_block_id() << ": " << unroll_sp << endl;
		//po->set_unrolled_sp(unroll_sp);
		it->second->set_unrolled_sp(unroll_sp);
		cout << "SET UNROLL SP: " << po->get_unrolled_sp()  << endl;




	}
}

Constraint ComputePO::initialize_precond(Constraint precond, POEKey* key)
{
	Constraint new_pre = precond;
	Block* b = key->getBlock();
	cout << "context: " << var_prefix << endl;
	const set<VariableTerm*>& mod_set = ms.get_modset(b);
	for(auto it = mod_set.begin(); it!= mod_set.end(); it++) {
		VariableTerm* cur = *it;
		if(cur->to_string().find("__temp") != string::npos) continue;
		Term* t = VariableTerm::make(var_prefix + cur->get_name());
		cur = static_cast<VariableTerm*>(t);
		Term* cur0 = VariableTerm::make(cur->get_name() + "$0");

		Constraint c(cur, cur0, ATOM_EQ);
		new_pre &= c;

	}
	return new_pre;
}

Constraint ComputePO::add_version_number(Constraint c, POEKey* key,
		ProofObligation* po, int k)
{
	Constraint new_c = c;
	Block* b = key->getBlock();
	cout << "context: " << var_prefix << endl;
	const set<VariableTerm*>& mod_set = ms.get_modset(b);
	map<VariableTerm*, VariableTerm*> mappings;
	for(auto it = mod_set.begin(); it!= mod_set.end(); it++) {
		VariableTerm* cur = *it;
		if(cur->to_string().find("__temp") != string::npos) continue;
		Term* t = VariableTerm::make(var_prefix + cur->get_name());
		cur = static_cast<VariableTerm*>(t);
		Term* new_cur = VariableTerm::make(cur->get_name() + "$" + int_to_string(k));
		new_c.replace_term(cur, new_cur);
		mappings[cur] = static_cast<VariableTerm*>(new_cur);


	}
	po->add_unroll_renaming(mappings);
	return new_c;
}


void ComputePO::compute_all_observed_and_I()
{

	map<POEKey*, ProofObligation*>::iterator mapBlock_iter = map_poekey_po.begin();
	for(; mapBlock_iter != map_poekey_po.end(); mapBlock_iter++)
	{
		if(mapBlock_iter->first->is_superblock())
		{
			SuperBlock* sb = static_cast<SuperBlock*>(mapBlock_iter->first->getBlock());
			set<CfgEdge*> edges = sb->get_entry_block()->get_successors();
			set<CfgEdge*>::iterator edge_iter;

			Block * body_entry = NULL;
			for(edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter++)
			{
				body_entry = (*edge_iter)->get_target();
				if(body_entry->get_block_id() == sb->get_exit_block()->get_block_id())
				{
					continue;
				}
				else
				{
					break;
				}
			}
			if(body_entry->is_basicblock())
			{
				BasicBlock* bb_body = static_cast<BasicBlock*>(body_entry);
				set<Constraint> observedInfo;
				Constraint ivariantInfo;
				//POEKey* pk_body = new POEKey(bb_body, mapBlock_iter->first->getcallingcontext());
				POEKey* pk_body = new POEKey(bb_body, new CallingContext());
				//compute_observedInfo(bb_body, observedInfo);
				compute_observedInfo(pk_body, observedInfo, ivariantInfo);
				mapBlock_iter->second->setObserved(observedInfo);
				//add c-> assumption to invariant
				mapBlock_iter->second->setKnown_inv(
						!(mapBlock_iter->second->getWhile_condition())|ivariantInfo);
			}
		}
	}

}

void ComputePO::adjust_I_based_on_P()
{
	map<POEKey*, ProofObligation*>::iterator it = map_poekey_po.begin();
	for(; it != map_poekey_po.end(); it++)
	{
		POEKey* key = it->first;
		ProofObligation* po = it->second;
		Block* b = key->getBlock();
		//cout << "prefix : " << this->var_prefix << endl;
		if(!key->is_superblock()) continue;
		const set<VariableTerm*>& mod_set = ms.get_modset(b);

		//cout << "block id : " << key->to_string() << endl;
		set<VariableTerm*> mod_set_prefix;
		for(auto itt = mod_set.begin(); itt!= mod_set.end(); itt++) {
			VariableTerm* cur = *itt;
			if(cur->to_string().find("__temp") != string::npos) continue;
			Term* t = VariableTerm::make(var_prefix + cur->get_name());
			VariableTerm* vt = static_cast<VariableTerm*>(t);
			//cout << "variable term : " << vt->to_string() << endl;
			mod_set_prefix.insert(vt);
		}
		Constraint p_temp = po->getPrecondition();
		Constraint i = po->getKnown_inv();
		//cout << "p before eliminate : " << p_temp << endl;
		p_temp.eliminate_evars(mod_set_prefix);
		//cout << "p after eliminate : " << p_temp << endl;
		po->setKnown_inv(p_temp&i);
	}
}

int ComputePO::get_time()
{
	return time;
}

int ComputePO::get_unroll_time()
{
	return unroll_time;
}

void ComputePO::compute_observedInfo_helper(POEKey* pk_body_entry,
		set<Constraint>& observedInfo, Constraint& ivariantInfo)
{
	worklist.insert(pk_body_entry);

	while(worklist.size() > 0){
		POEKey* poekey_b = *worklist.begin();
		worklist.erase(poekey_b);
		Block* b_cur = poekey_b->getBlock();
		if(b_cur->is_basicblock()){
			//po_cur = new ProofObligation();
			BasicBlock* bb_cur = static_cast<BasicBlock*>(b_cur);
			vector<Instruction*> statements = bb_cur->get_statements();
			vector<Instruction*>::iterator iter;
			ProofObligation * po_cur = find_ProofObligation(poekey_b);
			Constraint phi = po_cur->getPrecondition();
			for (iter = statements.begin(); iter != statements.end(); ++iter) {
				Instruction* inst = (*iter);
				switch (inst->get_instruction_id()) {
				case BINOP:
				{
					Binop* binop = static_cast<sail::Binop*> (inst);
					Variable* v = binop->get_lhs();
					Symbol * sym1 = binop->get_first_operand();
					Symbol * sym2 = binop->get_second_operand();
					Term* vt = VariableTerm::make(v->get_var_name());
					Term* t1 = symbol_to_term(sym1);
					Term* t2 = symbol_to_term(sym2);
					//Term* rhs_term = NULL;
					il::binop_type op = binop->get_binop();

					switch (op) {
					case _EQ: {

						Constraint phi0 = phi;
						Constraint phi1 = phi;

						//get temp
						Term* temp = VariableTerm::make("sp_binop_temp");
						//cout<< "temp  " << temp->to_string() << endl;

						Term* t_c0 = ConstantTerm::make(0);
						Term* t_c1 = ConstantTerm::make(1);

						//v=e[v'/v]
						Constraint c_0(vt, t_c0, ATOM_EQ);
						Constraint c_1(vt, t_c1, ATOM_EQ);

						//p[v'/v]
						phi0.replace_term(vt, temp);
						phi1.replace_term(vt, temp);

						phi0 = phi0 & c_0;
						phi1 = phi1 & c_1;

						VariableTerm* tempv = static_cast<VariableTerm*>(temp);
						phi0.eliminate_evar(tempv);
						phi1.eliminate_evar(tempv);

						Constraint c_exp(t1, t2, ATOM_EQ);
						Constraint c_nexp(t1, t2, ATOM_NEQ);


						Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
						Constraint c_imp_2 = (c_exp | phi0);
						phi =  c_imp_1 & c_imp_2;
						break;

					}
					default: {
						//return 0;
					}
					}
					break;
				}
				case CAST:
				{
					Cast* inst_assignment = static_cast<sail::Cast*> (inst);
					if(DEBUG){
						cout << "\nphi before wp_cast " << phi << endl;
					}
					phi = sp_cast(inst_assignment, phi);
					if(DEBUG){
						cout << "phi after wp_cast " << phi << endl;
					}
					break;
				}
				case FUNCTION_CALL:
				{
					FunctionCall* functionCall = static_cast<sail::FunctionCall*> (inst);
					functionCall->get_function_name();

					string name = functionCall->get_function_name();
					string str_observed = OBSERVED;
					if(strcmp(name.c_str(), str_observed.c_str()) == 0)
					{
						vector<Symbol*>* syms = functionCall->get_arguments();
						if(syms->size() > 1)
						{
							assert(false);
						}
						Term* t1 = symbol_to_term(syms->at(0));
						Term* t2 = ConstantTerm::make(1);
						Constraint observed_c = Constraint(t1, t2, ATOM_EQ);
						phi = phi& observed_c;
						eliminate_temp(phi);
						observedInfo.insert(phi);
						//phi = new Constraint(true);
					}
					else
					{
						//return 0;
					}

					break;
				}
				case ASSUME:
				{
					Assume* assume = static_cast<sail::Assume*> (inst);
					Symbol* sym = assume->get_predicate();
					Term* t1 = symbol_to_term(sym);
					Term* t2 = ConstantTerm::make(1);
					Constraint new_i_cons = Constraint(t1, t2, ATOM_EQ);
					phi = phi&new_i_cons;
					//phi = phi& observed_c;
					eliminate_temp(phi);
					ivariantInfo = ivariantInfo&phi;
					break;
				}

				default:{
					//return 0;
				}
				}
			}

		}

		set<CfgEdge*>& set_Edges = b_cur->get_successors();
		set <CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for (; iter_Edges!=set_Edges.end(); iter_Edges++)
		{
			POEKey * poekey_next = new POEKey((*iter_Edges)->get_target(),
					poekey_b->getcallingcontext());
			worklist.insert(poekey_next);
		}

	}
}

void ComputePO::compute_observedInfo(POEKey* pk_body_entry, set<Constraint>& observedInfo, Constraint& ivariantInfo)
{
	//ComputePO cpo_body(pk_body_entry, this->entry_func, this->func_vector, true);
	ComputePO cpo_body(pk_body_entry, this->entry_func, this->func_vector);
	if(DEBUG)
	{
		cout << "+++++++++++  compute_observedInfo " << endl;
	}
	cpo_body.compute_func_sp(pk_body_entry, NULL, true);
	cpo_body.compute_observedInfo_helper(pk_body_entry, observedInfo , ivariantInfo);
	if(DEBUG)
	{
		cout << "+++++++++++  end compute_observedInfo " << endl;
	}
}



void ComputePO::compute_all_superBlock_R()
{
	Constraint t(true);
	map<POEKey *, ProofObligation *>::iterator mapPOEKey_iter = map_poekey_po.begin();

	for(; mapPOEKey_iter != map_poekey_po.end(); mapPOEKey_iter++)
	{
		if(mapPOEKey_iter->first->is_superblock())
		{
			//SuperBlock* sb = static_cast<SuperBlock*>(mapPOEKey_iter->first->getBlock());
			Constraint wp_of_sb = compute_superBlock_wp(mapPOEKey_iter->first,
					t, func_vector, entry_func, false);
			//wp_of_sb.sat();
			mapPOEKey_iter->second->setInv_to_show(wp_of_sb);
		}
	}
}


Constraint ComputePO::compute_superBlock_wp(POEKey* sb_poekey, Constraint c,
		vector<Function *>& func_vector, Function* entry_func, bool is_body)
{
	//Function * f = NULL;
	Function * f = entry_func;
	CallingContext *calling_seq = sb_poekey->getcallingcontext();
	SuperBlock* sb = static_cast<SuperBlock*>(sb_poekey->getBlock());
	string func_name = calling_seq->get_curfunc_name();
	if(func_name != " ")//!func_name.empty())
	{
		vector<Function *>::iterator iter;
		for(iter = func_vector.begin(); iter != func_vector.end(); iter++)
		{
			function_declaration* declare = (*iter)->get_original_declaration();
			string name_declare =  declare->get_name();
			vector<variable_declaration*> args_declare = declare->get_arguments();
			if(std::strcmp(name_declare.c_str(), func_name.c_str()) == 0)
			{
				f = (*iter);
			}
		}
	}

	POEKey * entry_key = new POEKey(sb->get_entry_block(), calling_seq);
	ComputePO cpo_sb(entry_key, f, func_vector);
	return cpo_sb.compute_superBlock_wp_helper(sb, calling_seq ,c, is_body);

}


Constraint ComputePO::compute_superBlock_wp_helper(SuperBlock* sb,
		CallingContext *calling_seq,  Constraint cons, bool is_body)
{

	set <Block*> set_blocks = sb->get_body();
	set <Block*>::iterator set_blocks_iter = set_blocks.begin();
	for(; set_blocks_iter != set_blocks.end(); set_blocks_iter++)
	{
		if((*set_blocks_iter)->is_basicblock())
		{
			BasicBlock* bb = static_cast<BasicBlock *>(*set_blocks_iter);
			//ignore the empty block

			if(bb->get_statements().size() <= 0)
			{
				continue;
			}
			Instruction * inst = bb->get_statements().back();
			if(inst->get_instruction_id() == LOOP_INVOCATION)
			{
				POEKey *the_key = new POEKey(bb, calling_seq);
				ProofObligation * po_cur = find_ProofObligation(the_key);

				if(po_cur!= NULL){
					po_cur->add_constraint(cons, false);
					worklist.insert(the_key);
				}else
				{
					assert(false);
				}
			}

		}
		//it doesn't matter, if the block is a super block  (nested loop)
	}

	//put exit block in the queue
	BasicBlock * b_exit =  sb->get_exit_block();
	POEKey *exit_key = new POEKey(b_exit, calling_seq);
	ProofObligation * po_exit = find_ProofObligation(exit_key);
	po_exit->add_constraint(cons, false);
	//q_block.push(b_exit);
	worklist.insert(exit_key);


	set<CfgEdge*> edges = sb->get_entry_block()->get_successors();
	set<CfgEdge*>::iterator edge_iter;

	Block * body_entry = NULL;
	for(edge_iter = edges.begin(); edge_iter != edges.end(); edge_iter++)
	{
		body_entry = (*edge_iter)->get_target();
		if(body_entry->get_block_id() == sb->get_exit_block()->get_block_id())
		{
			continue;
		}
		else
		{
			break;
		}
	}


	// true means forward
	//the second argument is for extra_q_info.
	POEKey *key_body_entry = new POEKey(body_entry, calling_seq);
	POEKey *key_entry = new POEKey(sb->get_entry_block(), calling_seq);
	if(is_body)
	{
		compute_block_swp(false, key_body_entry, false);
	}
	else
	{
		compute_block_swp(false, key_entry, false);
	}
	return extra_above_info;

}


Constraint ComputePO::get_block_spInfo(POEKey *key){
	ProofObligation * b_po = find_ProofObligation(key);
	//	cout << "Finding proof obligation for key: " << key->to_string() << endl;
	//	cout << "Pre: " << b_po->getPrecondition() << endl;
	//	cout << "Post: " << b_po->getPostcondition() << endl;
	//	return b_po->getPostcondition();
	return b_po->getSp_after();
}

Constraint ComputePO::get_block_wpInfo(POEKey *key){
	ProofObligation * b_po = find_ProofObligation(key);
	return b_po->getPrecondition();
}


void ComputePO::compute_func_sp(POEKey *poekey_entry,
		POEKey *poekey_exit, Constraint phi)
{

	//put the entry block in the queue
	ProofObligation * po_cur = find_ProofObligation(poekey_entry);
	po_cur->add_constraint(phi, true);
	worklist.insert(poekey_entry);

	// true means forward
	// the second segment is not very useful.
	compute_block_swp(true, poekey_exit, true);

}


void ComputePO::compute_func_wp(POEKey *poekey_entry,
		POEKey *poekey_exit, Constraint phi)
{

	//put the exit block in the queue
	ProofObligation * po_cur = find_ProofObligation(poekey_exit);
	po_cur->add_constraint(phi, false);
	worklist.insert(poekey_exit);

	// true means forward
	compute_block_swp(false, poekey_entry, true);

}


void ComputePO::recompute_block_wp(Constraint added_phi,
		bool is_first_while, int line_num, LoopTree& lt)
{
	bool set_prefix = false;
	bool is_first_bb = true;
	while(worklist.size() > 0){
		POEKey* poekey_b = *worklist.rbegin();
		worklist.erase(poekey_b);

		if(!set_prefix)
		{
			CallingContext* calling_seq = poekey_b->getcallingcontext();
			//cout << "get get_curfunc_name  :  " << calling_seq->get_curfunc_name() << endl;
			if( calling_seq->get_curfunc_name() == " ")
			{
				function_declaration* declare = this->entry_func->get_original_declaration();
				var_prefix = declare->get_name()+"::";
			}else
			{
				this->var_prefix = calling_seq->get_curfunc_name() + "::";
			}
			set_prefix = true;
		}

		ProofObligation* po_cur = find_ProofObligation(poekey_b);
		if(DEBUG)
		{
			cout << "wp    ";
			cout << "#########################block id: "
					<< poekey_b->getBlock()->get_block_id() <<"#############################"<<endl;
		}
		Constraint phi;
		phi = po_cur->getPostcondition();


		//compute current block

		if(poekey_b->is_basicblock())
		{
			if(!is_first_bb || line_num == -1){
				is_first_bb = false;
				BasicBlock* bb = static_cast<BasicBlock *>(poekey_b->getBlock());
				vector<Instruction*> statements = bb->get_statements();
				phi = wp(statements, phi);
			}
			else
			{
				is_first_bb = false;
				phi = added_phi;
				if(entry_func!=NULL)
				{
					function_declaration* declare = this->entry_func->get_original_declaration();
					var_prefix = declare->get_name()+"::";

				}
				else
				{
					var_prefix = " ::";
				}
				BasicBlock* bb = static_cast<BasicBlock *>(poekey_b->getBlock());
				vector<Instruction*> statements_all = bb->get_statements();
				vector<Instruction*> statements;
				vector<Instruction*>::iterator iter_stat;
				for(iter_stat = statements_all.begin(); iter_stat != statements_all.end(); iter_stat++)
				{
					if((*iter_stat)->line == line_num)
					{
						break;
					}else
					{
						statements.push_back(*iter_stat);
					}
				}
				phi = wp(statements, phi);

			}
		}else{
			if(is_first_while)
			{
				phi = added_phi;
				is_first_while = false;
				Constraint pre = po_cur->getPrecondition();
				po_cur->setPrecondition(added_phi&pre);
				if(DEBUG)
				{
					cout << "phi : "  << phi << endl;
				}
			}else{
				//not first superblock
				continue;
			}
		}

		/*
		 * set wp before and eli
		 */
		po_cur->setWp_before(phi);

		set <CfgEdge*> set_Edges = poekey_b->get_predecessors();
		set <CfgEdge*>::iterator iter_Edges;
		for (iter_Edges=set_Edges.begin(); iter_Edges!=set_Edges.end(); iter_Edges++)
		{

			Block * b_next = (*iter_Edges)->get_source();
			POEKey * poekey_next = new POEKey(b_next, poekey_b->getcallingcontext());

			//branch or not branch
			Symbol * s = (*iter_Edges)->get_cond();
			Constraint new_phi;
			if(s == NULL){
				new_phi = phi;
			}else{
				Term* new_t = symbol_to_term(s);
				Term* t_c1 = ConstantTerm::make(1);
				Constraint cc = Constraint(new_t, t_c1, ATOM_EQ);
				new_phi = !cc |phi;
			}

			ProofObligation * po_next = find_ProofObligation(poekey_next);

			po_next->add_constraint(new_phi, false);
			worklist.insert(poekey_next);
		}

		if(worklist.size() == 0)
		{
			CallingContext* cc_entry = new CallingContext();
			POEKey* pk_entry = new POEKey(this->entry_func->get_entry_block(), cc_entry);

			if(*poekey_b == *pk_entry)
			{
				continue;
			}


			/*
			 * entry block of the while loop or entry block of the function call in below
			 */
			CallingContext* cur_cc = poekey_b->getcallingcontext();
			string cur_name = cur_cc->get_curfunc_name();
			if(cur_name == " ")
			{
				function_declaration* declare = this->entry_func->get_original_declaration();
				cur_name = declare->get_name();
			}

			Function* func_cur = function_finder(cur_name);
			POEKey* pk_cur_fun_entry = new POEKey(func_cur->get_entry_block(), cur_cc);

			/*
			 * Interprocedural analysis update
			 */
			//if(set_Edges.empty() && cur_cc->get_context_size() != 0)
			if(*poekey_b == *pk_cur_fun_entry && cur_cc->get_context_size() != 0)
			{
				/*
				 * find the caller
				 */
				//string cur_name = cur_cc->get_curfunc_name();
				int cur_line = cur_cc->get_back_line_num();
				// caller's cc
				CallingContext* copy_cc = new CallingContext(*cur_cc);
				copy_cc->pop_back_context();
				map<POEKey*, ProofObligation*>::iterator map_key_iter;
				for(map_key_iter = map_poekey_po.begin(); map_key_iter != map_poekey_po.end(); map_key_iter++)
				{
					Block* b = (*map_key_iter).first->getBlock();
					CallingContext* cc = (*map_key_iter).first->getcallingcontext();
					if(!(*cc == *copy_cc))
					{
						continue;
					}
					if(b->is_basicblock())
					{
						BasicBlock *bb_cur = static_cast<BasicBlock *> (b);
						vector<Instruction*> s = bb_cur->get_statements();
						vector<Instruction*>::iterator inst_iter;
						for (inst_iter = s.begin(); inst_iter != s.end(); ++inst_iter) {
							if((*inst_iter)->get_instruction_id() == FUNCTION_CALL)
							{
								FunctionCall* functionCall = static_cast<sail::FunctionCall*> (*inst_iter);
								string name = functionCall->get_function_name();
								if(functionCall->line == cur_line && strcmp(name.c_str(), cur_name.c_str()) == 0)
								{

									Constraint new_phi = phi;

									//callee's args
									vector<Term*> callee_args;
									string callee_func_name = cur_cc->get_curfunc_name();
									Function* func_callee = function_finder(callee_func_name);
									string func_caller_name;
									function_declaration* declare_callee = func_callee->get_original_declaration();
									string name_declare_callee =  declare_callee->get_name();
									vector<variable_declaration*> args_declare = declare_callee->get_arguments();
									for(unsigned int i = 0; i < args_declare.size(); i++)
									{
										Term *term = VariableTerm::make(name_declare_callee+"::"+args_declare[i]->get_variable()->get_name());
										callee_args.push_back(term);
									}

									vector<Symbol*>* syms = functionCall->get_arguments();
									vector<Symbol*>::iterator sym_iter;

									//caller's args
									vector<Term*> caller_args ;
									for(sym_iter = (*syms).begin(); sym_iter != (*syms).end(); sym_iter++)
									{
										Term * term = NULL;
										if ((*sym_iter)->is_variable()) {
											Variable* v = static_cast<Variable *> (*sym_iter);

											if(copy_cc->get_context_size() > 0)
											{
												func_caller_name = copy_cc->get_caller_name();
												term = VariableTerm::make(func_caller_name + "::" + v->get_var_name());
											}else
											{
												//function_declaration* declare_caller =  org_func->get_original_declaration();
												function_declaration* declare_caller =  this->entry_func->get_original_declaration();
												func_caller_name = declare_caller->get_name();
												term = VariableTerm::make(func_caller_name + "::" + v->get_var_name());
											}
										}
										caller_args.push_back(term);
									}

									//Function* func_caller = function_finder(func_caller_name);

									vector<Term*>::iterator callee_args_iter = callee_args.begin();
									vector<Term*>::iterator caller_args_iter = caller_args.begin();
									for(; callee_args_iter!=callee_args.end(); callee_args_iter++)
									{
										new_phi.replace_term(*callee_args_iter, *caller_args_iter);
										caller_args_iter++;
									}

									worklist.insert((*map_key_iter).first);
									line_num = cur_line;
									added_phi = new_phi;
									is_first_bb = true;
									set_prefix = false;
									//ComputePO cpo = ComputePO(func_caller, func_vector);
									//cpo.updatePre_helper((*map_key_iter).first, new_phi, map_poekey_po, org_func, cur_line);

								}
							}
						}
					}
				}
			}else
			{
				/*
				 * poekey_b is entry block of the while loop.
				 */
				BasicBlock* func_cur_entry = func_cur->get_entry_block();


				/*
				 * poekey_b is a while loop entry, then we should find the Superblock
				 */
				SuperBlock* sb_p = find_superblock(poekey_b->get_block_id() , func_cur_entry);
				POEKey* pk_sb = new POEKey(sb_p, cur_cc);
				ProofObligation* po_pk_sb = find_ProofObligation(pk_sb);


				/**************************************************************
				 *
				 *find body entry's wp, then add it to R of the loop
				 */
				set<CfgEdge*> edges = sb_p->get_entry_block()->get_successors();

				Block * body_entry = NULL;
				set<CfgEdge*>::iterator edge_iter = edges.begin();
				for(; edge_iter != edges.end(); edge_iter++)
				{
					body_entry = (*edge_iter)->get_target();
					if(body_entry->get_block_id() == sb_p->get_exit_block()->get_block_id())
					{
						continue;
					}
					else
					{
						break;
					}
				}
				POEKey* pk_sb_body_entry = new POEKey(body_entry, cur_cc);
				ProofObligation * po_body_entry = find_ProofObligation(pk_sb_body_entry);

				//po_body_entry->getWp_before() is wp(s, phi)
				Constraint new_outer_R = po_body_entry->getWp_before()&po_pk_sb->getInv_to_show();
				po_pk_sb->setInv_to_show(new_outer_R);
				po_pk_sb->setWp_before(new_outer_R);


				list<POEKey*> list_inner;
				lt.getInnerLoopList(pk_sb, list_inner);

				list<POEKey*>::iterator list_riter;

				POEKey* pk_sb_entry = new POEKey(sb_p->get_entry_block(), cur_cc);
				ComputePO cpo = ComputePO(pk_sb_entry, entry_func, func_vector);
				cpo.sandwitch_loop_sp(pk_sb, new_outer_R);
				cpo.sandwitch_loop_wp(pk_sb, new_outer_R);
				cpo.updateInnerPO(map_poekey_po, list_inner);
			}
		}
	}
}



SuperBlock* ComputePO::find_superblock(int block_id , BasicBlock* func_entry)
{
	set<sail::Block*, sail::CompareBlock> f_worklist;

	//generate map
	f_worklist.insert(func_entry);

	while(f_worklist.size() > 0){
		sail::Block* b_cur = *f_worklist.begin();
		f_worklist.erase(b_cur);

		if(b_cur->is_basicblock()){
			// do nothing
		}else{
			SuperBlock* spb_cur = static_cast<SuperBlock *> (b_cur);
			if(spb_cur->get_block_id() == block_id)
			{
				return spb_cur;
			}
			BasicBlock* bb_sb_entry = spb_cur->get_entry_block();
			//Add entry of nested while loop into the worklist
			f_worklist.insert(bb_sb_entry);
		}

		set<CfgEdge*>& set_Edges = b_cur->get_successors();
		set <CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for (; iter_Edges!=set_Edges.end(); iter_Edges++)
		{
			f_worklist.insert((*iter_Edges)->get_target());
		}
	}
	return NULL;
}



void ComputePO::updatePre(POEKey* w, Constraint phi, map<POEKey*, ProofObligation*>& mapPOEKey, LoopTree& lt)
{
	this->map_poekey_po = mapPOEKey;
	worklist.insert(w);
	recompute_block_wp(phi, true, -1, lt);
}



void ComputePO::eliminate_temp(Constraint & cons)
{
	set<Term*> all_terms;
	cons.get_terms(all_terms, false);
	set<VariableTerm*> eleminate_terms;
	set<Term*>::iterator iter_terms = all_terms.begin();
	for(; iter_terms != all_terms.end(); iter_terms++)
	{
		set<string> string_vars;


		if((*iter_terms)->type == VARIABLE_TERM)
		{
			//Does term have multiple variables?
			(*iter_terms)->get_vars(string_vars);
			set<string>::iterator iter_string = string_vars.begin();
			for(; iter_string != string_vars.end(); iter_string++)
			{
				size_t found;

				found = (*iter_string).find("__temp");
				if (found!=string::npos)
				{
					VariableTerm* vt = static_cast<VariableTerm*>(*iter_terms);
					eleminate_terms.insert(vt);
				}
			}
		}
	}

	set<VariableTerm*>::iterator iter_vt;
	//	for(iter_vt = eleminate_terms.begin(); iter_vt != eleminate_terms.end(); iter_vt++)
	//	{
	//		cout << (*iter_vt)->to_string() << endl;
	//	}
	cons.eliminate_evars(eleminate_terms);
}


void ComputePO::eliminate_temp(Constraint & cons, Term* t)
{
	set<Term*> all_terms;
	cons.get_terms(all_terms, false);
	set<VariableTerm*> eleminate_terms;
	set<Term*>::iterator iter_terms = all_terms.begin();
	for(; iter_terms != all_terms.end(); iter_terms++)
	{
		set<string> string_vars;
		set<string> string_vars2;
		if((*iter_terms)->type == VARIABLE_TERM)
		{
			//Does term have multiple variables?
			(*iter_terms)->get_vars(string_vars);
			set<string>::iterator iter_string = string_vars.begin();
			for(; iter_string != string_vars.end(); iter_string++)
			{
				t->get_vars(string_vars2);
				assert(string_vars2.size() == 1);
				if((*iter_string)== (*string_vars2.begin()))
					continue;
				size_t found;
				found = (*iter_string).find("__temp");
				if (found!=string::npos)
				{
					VariableTerm* vt = static_cast<VariableTerm*>(*iter_terms);
					eleminate_terms.insert(vt);
				}
			}
		}
	}

	set<VariableTerm*>::iterator iter_vt;
	cons.eliminate_evars(eleminate_terms);
}





void ComputePO::get_map_poekey_po(map<POEKey*, ProofObligation*>  & map)
{
	map = this->map_poekey_po;
}


void ComputePO::set_entry_func(Function * entry_f)
{
	this->entry_func = entry_f;
}

Constraint ComputePO::get_extra_after_info()
{
	return this->extra_after_info;
}


Constraint ComputePO::get_extra_above_info()
{
	return this->extra_above_info;
}



//-------------------weakest precondition part ----------------------

Constraint ComputePO::wp(vector<Instruction*>& s, Constraint phi) {
	if(DEBUG){
		cout << "before ----> phi: " << phi << endl;
	}

	vector<Instruction*>::reverse_iterator iter;
	for (iter = s.rbegin(); iter != s.rend(); iter++) {
		//cout << (*iter)->line << ": " << (*iter)->to_string()<< endl;
		phi = wphelper((*iter), phi);
		phi.sat();
	}
	if(DEBUG){
		cout << "after ----> phi: " << phi << endl;
	}
	return phi;
}


Constraint ComputePO::wphelper(Instruction* inst, Constraint phi) {

	//check the type of the instruction
	switch (inst->get_instruction_id()) {
	case ASSIGNMENT:
	{
		Assignment* inst_assignment = static_cast<sail::Assignment*> (inst);
		if(DEBUG){
			cout << "\nphi before wp_assignment " << phi << endl;
		}
		phi = wp_assignment(inst_assignment, phi);
		if(DEBUG){
			cout << "phi after wp_assignment " << phi << endl;
		}
		break;
	}
	case UNOP:
	{
		if(DEBUG){
			cout << "\nphi before wp_unop " << phi << endl;
		}
		Unop* inst_unop = static_cast<sail::Unop*> (inst);
		phi = wp_unop(inst_unop, phi);
		if(DEBUG){
			cout << "phi after wp_unop " << phi << endl;
		}
	}
	case BINOP:
	{
		if(DEBUG){
			cout << "\nphi before wp_binop " << phi << endl;
		}
		Binop* inst_binop = static_cast<sail::Binop*> (inst);
		phi = wp_binop(inst_binop, phi);
		if(DEBUG){
			cout << "phi after wp_binop " << phi << endl;
		}
		break;
	}
	case ASSUME:
	{
		Assume* inst_assume = static_cast<sail::Assume*> (inst);
		phi = wp_assume(inst_assume, phi);
		break;
	}
	case STATIC_ASSERT:
	{
		StaticAssert* assertion = static_cast<sail::StaticAssert*> (inst);
		if(DEBUG){
			cout << "\nphi before assert " << phi << endl;
		}
		phi = wp_assert(assertion, phi);
		if(DEBUG){
			cout << "phi after assert " << phi << endl;
		}
		break;
	}
	case ADDRESS_LABEL:
	{
		assert(false);
		break;
	}
	case ADDRESS_VAR:
	{
		assert(false);
		break;
	}
	case ADDRESS_STRING:
	{
		assert(false);
		break;
	}
	case ARRAY_REF_READ:
	{
		assert(false);
		break;
	}
	case ARRAY_REF_WRITE:
	{
		assert(false);
		break;
	}
	case SAIL_ASSEMBLY:
	{
		assert(false);
		break;
	}
	case BRANCH:
	{
		assert(false);
		break;
	}
	case CAST:
	{
		Cast* inst_assignment = static_cast<sail::Cast*> (inst);
		if(DEBUG){
			cout << "\nphi before wp_cast " << phi << endl;
		}
		phi = wp_cast(inst_assignment, phi);
		if(DEBUG){
			cout << "phi after wp_cast " << phi << endl;
		}
		break;
	}
	case FIELD_REF_READ:
	{
		assert(false);
		break;
	}
	case FIELD_REF_WRITE:
	{
		assert(false);
		break;
	}
	case FUNCTION_CALL:
	{
		FunctionCall* functionCall = static_cast<sail::FunctionCall*> (inst);
		if(DEBUG){
			cout << "\nphi before functionCall " << phi << endl;
		}
		phi = wp_functionCall(functionCall, phi);
		if(DEBUG){
			cout << "phi after functionCall " << phi << endl;
		}
		break;
	}
	case FUNCTION_POINTER_CALL:
	{
		assert(false);
		break;
	}
	case JUMP:
	{
		assert(false);
		break;
	}
	case SAIL_LABEL:
	{
		assert(false);
		break;
	}
	case LOAD:
	{
		assert(false);
		break;
	}
	case STORE:
	{
		assert(false);
		break;
	}
	case LOOP_INVOCATION:
	{
		//trivial
		break;
	}
	case DROP_TEMPORARY:
	{
		assert(false);
		break;
	}
	case ASSUME_SIZE:
	{
		assert(false);
		break;
	}

	default:
		assert(false);
		break;
	}
	return phi;
}


Term* ComputePO::symbol_to_term(Symbol* sym) {
	Term * term = NULL;
	if (sym->is_variable()) {
		Variable* v = static_cast<Variable *> (sym);
		if(!v->is_global())
		{
			term = VariableTerm::make(var_prefix + v->get_var_name());
		}
		else
		{
			term = VariableTerm::make(v->get_var_name());
		}

	} else {
		Constant* c = static_cast<Constant *> (sym);
		term = ConstantTerm::make( c->get_integer());
	}
	return term;
}


Constraint ComputePO::wp_assignment(Assignment* assign, Constraint phi) {
	if(DEBUG){
		cout << assign->to_string() << endl;
	}

	//get old term
	Variable* v = assign->get_lhs();
	Term* old_t;
	if(!v->is_global())
	{
		old_t = VariableTerm::make(var_prefix +v->get_var_name());
	}
	else
	{
		old_t = VariableTerm::make(v->get_var_name());
	}

	//get new term
	Symbol* sym = assign->get_rhs();
	Term* new_t = symbol_to_term(sym);

	//replace and return
	phi.replace_term(old_t, new_t);

	return phi;
}


Constraint ComputePO::wp_cast(Cast* cast, Constraint phi) {
	if(DEBUG){
		cout << cast->to_string() << endl;
	}

	//get old term
	Variable* v = cast->get_lhs();
	Term* old_t;
	if(!v->is_global())
	{
		old_t = VariableTerm::make(var_prefix +v->get_var_name());
	}
	else
	{
		old_t = VariableTerm::make(v->get_var_name());
	}

	//get new term
	Symbol* sym = cast->get_rhs();
	Term* new_t = symbol_to_term(sym);

	//replace and return
	phi.replace_term(old_t, new_t);

	return phi;
}




Constraint ComputePO::wp_assume(Assume* inst_assume, Constraint phi){
	if(DEBUG){
		cout << inst_assume->to_string() << endl;
	}

	Symbol* pred = inst_assume->get_predicate();
	Constraint pred_c;
	if (pred->is_constant()) {
		Constant* c = static_cast<Constant *> (pred);
		if (c->get_integer() == 0) {
			pred_c = Constraint(false);
		} else {
			pred_c = Constraint(true);
		}
	} else {
		Variable * v = static_cast<Variable *> (pred);
		//BooleanVar * b = BooleanVar::make(v->get_var_name());
		//pred_c = Constraint(b);
		Term* t1;
		if(!v->is_global())
		{
			t1 = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			t1 = VariableTerm::make(v->get_var_name());
		}
		Term* t2 = ConstantTerm::make(0);
		pred_c = Constraint(t1, t2, ATOM_NEQ);
	}
	return !pred_c | phi;

	//return phi;
}


Constraint ComputePO::wp_assert(StaticAssert* assertion, Constraint phi) {
	if(DEBUG){
		cout << assertion->to_string() << endl;
	}

	Symbol* pred = assertion->get_predicate();
	Constraint pred_c;
	if (pred->is_constant()) {
		Constant* c = static_cast<Constant *> (pred);
		if (c->get_integer() == 0) {
			pred_c = Constraint(false);
		} else {
			pred_c = Constraint(true);
		}
	} else {
		Variable * v = static_cast<Variable *> (pred);
		//BooleanVar * b = BooleanVar::make(v->get_var_name());
		//pred_c = Constraint(b);
		Term* t1;
		if(!v->is_global())
		{
			t1 = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			t1 = VariableTerm::make(v->get_var_name());
		}
		Term* t2 = ConstantTerm::make(0);
		pred_c = Constraint(t1, t2, ATOM_NEQ);
	}
	return pred_c & phi;
}


Constraint ComputePO::wp_binop(Binop* binop, Constraint phi) {
	if(DEBUG){
		cout << binop->to_string() << endl;
	}

	Variable* v = binop->get_lhs();
	Symbol * sym1 = binop->get_first_operand();
	Symbol * sym2 = binop->get_second_operand();

	Term* vt;
	if(!v->is_global())
	{
		vt = VariableTerm::make(var_prefix +v->get_var_name());
	}
	else
	{
		vt = VariableTerm::make(v->get_var_name());
	}


	Term* t1 = symbol_to_term(sym1);
	Term* t2 = symbol_to_term(sym2);
	Term* rhs_term = NULL;
	il::binop_type op = binop->get_binop();

	switch (op) {
	case _PLUS: {
		rhs_term = t1->add(t2);
		phi.replace_term(vt, rhs_term);
		break;
	}
	case _MINUS: {
		rhs_term = t1->subtract(t2);
		phi.replace_term(vt, rhs_term);
		break;
	}
	case _MULTIPLY: {
		rhs_term = t1->multiply(t2);
		phi.replace_term(vt, rhs_term);
		break;
	}
	case _MOD: {
		if(sym2->is_constant())
		{
			Constant* c = static_cast<Constant *> (sym2);
			int cons = c->get_integer();
			phi = wp_mod_c(vt, t1, cons, phi);
			break;
		}
		else
		{
			//if op2 is variable
			vector<Term*> args;
			args.push_back(t1);
			args.push_back(t2);
			string name = var_prefix +"fun" + int_to_string(((int) op));
			rhs_term = FunctionTerm::make(name, args, false);
			phi.replace_term(vt, rhs_term);
			break;

		}
	}
	case _DIV: {
		vector<Term*> args;
		args.push_back(t1);
		args.push_back(t2);
		rhs_term = FunctionTerm::make(var_prefix +"divide", args, false);
		phi.replace_term(vt, rhs_term);
		break;
	}
	case _LT: {
		Constraint phi1 = phi;
		Constraint phi2 = phi;
		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);
		phi1.replace_term(vt, t_c1);
		phi2.replace_term(vt, t_c0);

		Constraint c_exp(t1, t2, ATOM_LT);
		Constraint c_nexp(t1, t2, ATOM_GEQ);

		Constraint c_imp_1 = (c_nexp | phi1);
		Constraint c_imp_2 = (c_exp | phi2);
		phi =  c_imp_1 & c_imp_2;
		break;

	}
	case _LEQ: {
		Constraint phi1 = phi;
		Constraint phi2 = phi;
		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);
		phi1.replace_term(vt, t_c1);
		phi2.replace_term(vt, t_c0);

		Constraint c_exp(t1, t2, ATOM_LEQ);
		Constraint c_nexp(t1, t2, ATOM_GT);

		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi2);
		phi =  c_imp_1 & c_imp_2;
		break;

	}
	case _GT: {
		Constraint phi1 = phi;
		Constraint phi2 = phi;
		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);
		phi1.replace_term(vt, t_c1);
		phi2.replace_term(vt, t_c0);

		Constraint c_exp(t1, t2, ATOM_GT);


		Constraint c_imp_1 = (!c_exp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi2);
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _GEQ: {
		Constraint phi1 = phi;
		Constraint phi2 = phi;
		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);
		phi1.replace_term(vt, t_c1);
		phi2.replace_term(vt, t_c0);

		Constraint c_exp(t1, t2, ATOM_GEQ);
		Constraint c_nexp(t1, t2, ATOM_LT);

		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi2);
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _EQ: {
		Constraint phi1 = phi;
		Constraint phi2 = phi;
		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);
		phi1.replace_term(vt, t_c1);
		phi2.replace_term(vt, t_c0);

		Constraint c_exp(t1, t2, ATOM_EQ);
		Constraint c_nexp(t1, t2, ATOM_NEQ);

		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi2);
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _NEQ: {
		Constraint phi1 = phi;
		Constraint phi2 = phi;
		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);
		phi1.replace_term(vt, t_c1);
		phi2.replace_term(vt, t_c0);

		Constraint c_exp(t1, t2, ATOM_NEQ);
		Constraint c_nexp(t1, t2, ATOM_EQ);

		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi2);
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _LOGICAL_AND: {
		assert(false);
		break;
	}
	case _LOGICAL_OR: /* &&, || (shortcircuit semantics) */
	{
		assert(false);
		break;
	}
	case _LOGICAL_AND_NO_SHORTCIRCUIT: {
		assert(false);
		break;
	}
	case _LOGICAL_OR_NO_SHORTCIRCUIT: {
		assert(false);
		break;
	}
	default: {
		vector<Term*> args;
		args.push_back(t1);
		args.push_back(t2);
		string name = var_prefix +"fun" + int_to_string(((int) op));
		rhs_term = FunctionTerm::make(name, args, false);
		phi.replace_term(vt, rhs_term);
		break;
	}
	}
	return phi;

}


Constraint ComputePO::wp_functionCall(FunctionCall* functionCall, Constraint phi)
{
	if(DEBUG){
		cout << functionCall->to_string() << endl;
	}
	Constraint phi_temp = phi;

	Variable* v = functionCall->get_lhs();

	/*
	 * deal with unknown function
	 */
	string name = functionCall->get_function_name();
	size_t found;
	found = name.find(UNKNOWN_STR);
	size_t found2;
	found2 = name.find(OBSERVED);
	if (found!=string::npos)
	{
		if(v == NULL)
		{
			//lhs of unknown function can never be NULL
			assert(false);
		}
		//args of unknow function is always empty
		vector<Term*> args ;

		Term* new_term = VariableTerm::make("__" + name);
		Term* vt = VariableTerm::make(this->var_prefix + v->get_var_name());
		phi.replace_term(vt, new_term);
		return phi;
	} else if(found2!=string::npos)
	{
		vector<Term*> args;
		Term* rhs_term = FunctionTerm::make(name, args, false);
		Term* vt = VariableTerm::make(this->var_prefix + v->get_var_name());
		phi.replace_term(vt, rhs_term);
		return phi;

	}else
	{

		/*
		 * I. Handle return values. Use assignment.
		 */
		if(v != NULL)
		{
			Term* old_t = VariableTerm::make(var_prefix + v->get_var_name());
			Term* new_t = VariableTerm::make(name + "::__return");
			phi_temp.replace_term(old_t, new_t);
		}

		Function * callingfunc = function_finder(name);//, args);
		CallingContext* cc = new CallingContext(*(cur_eval_key->getcallingcontext()));
		cc->add_context(this->entry_func, functionCall->line);
		POEKey* poekey_func_entry = new POEKey(callingfunc->get_entry_block(), cc);
		POEKey* poekey_func_exit = new POEKey(callingfunc->get_exit_block(), cc);
		if(DEBUG)
		{
			cout << "************************   wp sub function " << name << " has been called ***********************" << endl;
		}
		Constraint c_after_wp = wp_sub_function(poekey_func_entry, poekey_func_exit, phi_temp , callingfunc);
		if(DEBUG)
		{
			cout << "************************   sub function calling end ***********************" << endl;
		}
		phi_temp = c_after_wp;

		/*
		 * I. mapping: caller -->  callee
		 */
		vector<Symbol*>* syms = functionCall->get_arguments();
		vector<Symbol*>::iterator iter;
		vector<Term*> args;
		for(iter = (*syms).begin(); iter != (*syms).end(); iter++)
		{
			args.push_back(symbol_to_term(*iter));
		}
		//string name = functionCall->get_function_name();

		function_declaration* declare = callingfunc->get_original_declaration();
		string name_declare =  declare->get_name();
		vector<variable_declaration*> args_declare = declare->get_arguments();

		for(unsigned int i = 0; i < args_declare.size(); i++)
		{
			Term *term = VariableTerm::make(name_declare+"::"+ args_declare[i]->get_variable()->get_name());
			phi_temp.replace_term(term, args[i]);
		}
		return phi_temp;
	}
}



Constraint ComputePO::wp_sub_function(POEKey* poekey_func_entry, POEKey* poekey_func_exit, Constraint begin,Function* callingfunc)
{
	ComputePO* newCPO = new ComputePO(callingfunc, this->map_poekey_po, this->func_vector);
	//cout << "poekey_func_entry   :" << poekey_func_entry->to_string() << "poekey_func_exit   : " << poekey_func_exit->to_string() << endl;
	newCPO->compute_func_wp(poekey_func_entry, poekey_func_exit, begin);
	newCPO->get_map_poekey_po(this->map_poekey_po);
	return newCPO->get_extra_above_info();
}







Constraint ComputePO::wp_unop(Unop* unop, Constraint phi) {
	if(DEBUG){
		cout << unop->to_string() << endl;
	}
	il::unop_type op = unop->get_unop();

	switch (op) {
	case _NEGATE: {
		//can put it before switch
		Variable* v = unop->get_lhs();
		Symbol * sym = unop->get_operand();
		Term* vt;
		if(!v->is_global())
		{
			vt = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			vt = VariableTerm::make(v->get_var_name());
		}


		Term* t1 = symbol_to_term(sym);
		Term* rhs_term = NULL;

		rhs_term = t1->flip_sign();
		phi.replace_term(vt, rhs_term);
		break;
	}
	case _LOGICAL_NOT: /* !x */
	{

		Variable* v = unop->get_lhs();
		Symbol * sym = unop->get_operand();
		Term* vt;
		if(!v->is_global())
		{
			vt = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			vt = VariableTerm::make(v->get_var_name());
		}
		Term* t1 = symbol_to_term(sym);

		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);

		Constraint phi0 = phi;
		Constraint phi1 = phi;

		phi0.replace_term(vt, t_c0);
		phi1.replace_term(vt, t_c1);

		Constraint c_exp(t1, t_c0, ATOM_NEQ);

		//t=!x

		Constraint c_imp_1 = (!c_exp | phi0);// x!=0  implies t = 0;
		Constraint c_imp_2 = (c_exp | phi1); // x=0  implies t = 1;
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _PREDECREMENT:
	{
		assert(false);
		break;
	}
	case _PREINCREMENT:  /* --i, ++i */
	{
		assert(false);
		break;
	}
	case _POSTDECREMENT:
	{
		assert(false);
		break;
	}
	case _POSTINCREMENT: /* i--, i++ */
	{
		assert(false);
		break;
	}
	default: {
		//_CONJUGATE, /* complex conjugate */
		//_BITWISE_NOT, /* ~x */
		//_VEC_UNOP

		Variable* v = unop->get_lhs();
		Symbol * sym = unop->get_operand();
		Term* vt;
		if(!v->is_global())
		{
			vt = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			vt = VariableTerm::make(v->get_var_name());
		}
		Term* t1 = symbol_to_term(sym);
		Term* rhs_term = NULL;

		vector<Term*> args;
		args.push_back(t1);

		string name = var_prefix +"fun" + int_to_string(((int) op));
		rhs_term = FunctionTerm::make(name, args, false);
		phi.replace_term(vt, rhs_term);
		break;
	}
	}
	return phi;
}


Constraint ComputePO::wp_mod_c(Term* vt, Term* t1, int cons, Constraint phi){
	//if op2 is constant

	Term * term_c_0 = ConstantTerm::make(0);
	Term * term_c_1 = ConstantTerm::make(1);
	Term * term_c_2 = ConstantTerm::make(2);
	Term * term_c_3 = ConstantTerm::make(3);
	Term * term_c_4 = ConstantTerm::make(4);

	Term * term_cons = ConstantTerm::make(cons);

	switch (cons) {
	case 2:
	{

		Constraint phi0 = phi;
		Constraint phi1 = phi;

		phi0.replace_term(vt, term_c_0);
		phi1.replace_term(vt, term_c_1);

		Term * t_mod_eq_0 = t1;
		Constraint c_mod_eq_0(t_mod_eq_0, term_cons, ATOM_MOD);

		Term * t_mod_eq_1 = t1->subtract(term_c_1);
		Constraint c_mod_eq_1(t_mod_eq_1, term_cons, ATOM_MOD);

		phi = (!c_mod_eq_0 | phi0);
		phi = phi &(!c_mod_eq_1 | phi1);

		break;
	}
	case 3:
	{
		Constraint phi0 = phi;
		Constraint phi1 = phi;
		Constraint phi2 = phi;

		phi0.replace_term(vt, term_c_0);
		phi1.replace_term(vt, term_c_1);
		phi1.replace_term(vt, term_c_2);


		Term * t_mod_eq_0 = t1;
		Constraint c_mod_eq_0(t_mod_eq_0, term_cons, ATOM_MOD);

		Term * t_mod_eq_1 = t1->subtract(term_c_1);
		Constraint c_mod_eq_1(t_mod_eq_1, term_cons, ATOM_MOD);


		Term * t_mod_eq_2 = t1->subtract(term_c_2);
		Constraint c_mod_eq_2(t_mod_eq_2, term_cons, ATOM_MOD);

		phi = (!c_mod_eq_0 | phi0);
		phi = phi &(!c_mod_eq_1 | phi1);
		phi = phi &(!c_mod_eq_2 | phi2);

		break;
	}
	case 4:
	{
		Constraint phi0 = phi;
		Constraint phi1 = phi;
		Constraint phi2 = phi;
		Constraint phi3 = phi;

		phi0.replace_term(vt, term_c_0);
		phi1.replace_term(vt, term_c_1);
		phi1.replace_term(vt, term_c_2);
		phi1.replace_term(vt, term_c_3);


		Term * t_mod_eq_0 = t1;
		Constraint c_mod_eq_0(t_mod_eq_0, term_cons, ATOM_MOD);

		Term * t_mod_eq_1 = t1->subtract(term_c_1);
		Constraint c_mod_eq_1(t_mod_eq_1, term_cons, ATOM_MOD);

		Term * t_mod_eq_2 = t1->subtract(term_c_2);
		Constraint c_mod_eq_2(t_mod_eq_2, term_cons, ATOM_MOD);

		Term * t_mod_eq_3 = t1->subtract(term_c_3);
		Constraint c_mod_eq_3(t_mod_eq_3, term_cons, ATOM_MOD);


		phi = (!c_mod_eq_0 | phi0);
		phi = phi &(!c_mod_eq_1 | phi1);
		phi = phi &(!c_mod_eq_2 | phi2);
		phi = phi &(!c_mod_eq_3 | phi3);

		break;
	}
	case 5:
	{
		Constraint phi0 = phi;
		Constraint phi1 = phi;
		Constraint phi2 = phi;
		Constraint phi3 = phi;
		Constraint phi4 = phi;

		phi0.replace_term(vt, term_c_0);
		phi1.replace_term(vt, term_c_1);
		phi1.replace_term(vt, term_c_2);
		phi1.replace_term(vt, term_c_3);
		phi1.replace_term(vt, term_c_4);


		Term * t_mod_eq_0 = t1;
		Constraint c_mod_eq_0(t_mod_eq_0, term_cons, ATOM_MOD);

		Term * t_mod_eq_1 = t1->subtract(term_c_1);
		Constraint c_mod_eq_1(t_mod_eq_1, term_cons, ATOM_MOD);

		Term * t_mod_eq_2 = t1->subtract(term_c_2);
		Constraint c_mod_eq_2(t_mod_eq_2, term_cons, ATOM_MOD);

		Term * t_mod_eq_3 = t1->subtract(term_c_3);
		Constraint c_mod_eq_3(t_mod_eq_3, term_cons, ATOM_MOD);

		Term * t_mod_eq_4 = t1->subtract(term_c_4);
		Constraint c_mod_eq_4(t_mod_eq_4, term_cons, ATOM_MOD);


		phi = (!c_mod_eq_0 | phi0);
		phi = phi &(!c_mod_eq_1 | phi1);
		phi = phi &(!c_mod_eq_2 | phi2);
		phi = phi &(!c_mod_eq_3 | phi3);
		phi = phi &(!c_mod_eq_4 | phi4);

		break;
	}
	default: {
		//if op2 is variable
		vector<Term*> args;
		args.push_back(t1);
		args.push_back(term_cons);
		string name = "fun_MOD";
		Term* rhs_term = FunctionTerm::make(name, args, false);
		phi.replace_term(vt, rhs_term);
		break;
	}
	}
	return phi;
}


//-------------------strongest postcondition part ----------------------

Constraint ComputePO::sp(vector<Instruction*>& s, Constraint phi) {
	if(DEBUG){
		cout << "----> phi: " << phi << endl;
	}

	//foreach Instruction call sphelper
	vector<Instruction*>::iterator iter;
	for (iter = s.begin(); iter != s.end(); ++iter) {
		//cout << (*iter)->line << "id:" << (*iter) ->to_string() << endl;
		phi = sphelper((*iter), phi);
		phi.sat();
	}

	if(DEBUG){
		cout << endl << "phi ******* "  <<  phi.to_string() << endl;
	}
	return phi;
}


Constraint ComputePO::sphelper(Instruction* inst, Constraint phi) {
	//check the type of the instruction
	switch (inst->get_instruction_id()) {
	case ASSIGNMENT: {
		Assignment* inst_assignment = static_cast<sail::Assignment*> (inst);
		if(DEBUG){
			cout << "\nphi before sp_assignment " << phi << endl;
		}
		phi = sp_assignment(inst_assignment, phi);
		if(DEBUG){
			cout << "phi after sp_assignment " << phi << endl;
		}
		break;
	}
	case UNOP: {
		if(DEBUG){
			cout << "\nphi before sp_unop " << phi << endl;
		}
		Unop* inst_unop = static_cast<sail::Unop*> (inst);
		phi = sp_unop(inst_unop, phi);
		if(DEBUG){
			cout << "phi aftersp_unop " << phi << endl;
		}
		break;
	}
	case BINOP: {
		if(DEBUG){
			cout << "\nphi before sp_binop " << phi << endl;
		}
		Binop* inst_binop = static_cast<sail::Binop*> (inst);
		phi = sp_binop(inst_binop, phi);
		if(DEBUG){
			cout << "phi after sp_binop " << phi << endl;
		}
		break;
	}
	case ASSUME: {
		Assume* inst_assume = static_cast<sail::Assume*> (inst);
		phi = sp_assume(inst_assume, phi);
		break;
	}
	case STATIC_ASSERT: {
		StaticAssert* assertion = static_cast<sail::StaticAssert*> (inst);
		if(DEBUG){
			cout << "\nphi before sp_assert " << phi << endl;
		}
		phi = sp_assert(assertion, phi);
		if(DEBUG){
			cout << "phi after sp_assert " << phi << endl;
		}
		break;
	}
	case ADDRESS_LABEL:
	{
		assert(false);
		break;
	}
	case ADDRESS_VAR:
	{
		assert(false);
		break;
	}
	case ADDRESS_STRING:
	{
		assert(false);
		break;
	}
	case ARRAY_REF_READ:
	{
		assert(false);
		break;
	}
	case ARRAY_REF_WRITE:
	{
		assert(false);
		break;
	}
	case SAIL_ASSEMBLY:
	{
		assert(false);
		break;
	}
	case BRANCH:
	{
		assert(false);
		break;
	}
	case CAST:
	{
		Cast* inst_cast = static_cast<sail::Cast*> (inst);
		if(DEBUG){
			cout << "\nphi before sp_cast " << phi << endl;
		}
		phi = sp_cast(inst_cast, phi);
		if(DEBUG){
			cout << "phi after sp_cast " << phi << endl;
		}
		break;
	}
	case FIELD_REF_READ:
	{
		assert(false);
		break;
	}
	case FIELD_REF_WRITE:
	{
		assert(false);
		break;
	}
	case FUNCTION_CALL:
	{
		FunctionCall* functionCall = static_cast<sail::FunctionCall*> (inst);
		if(DEBUG){
			cout << "\nphi before sp_functionCall " << phi << endl;
		}
		phi = sp_functionCall(functionCall, phi);
		if(DEBUG){
			cout << "phi after sp_functionCall " << phi << endl;
		}
		break;
	}
	case FUNCTION_POINTER_CALL:
	{
		assert(false);
		break;
	}
	case JUMP:
	{
		assert(false);
		break;
	}
	case SAIL_LABEL:
	{
		assert(false);
		break;
	}
	case LOAD:
	{
		assert(false);
		break;
	}
	case STORE:
	{
		assert(false);
		break;
	}
	case LOOP_INVOCATION:
	{
		//trivial
		break;
	}
	case DROP_TEMPORARY:
	{
		assert(false);
		break;
	}
	case ASSUME_SIZE:
	{
		assert(false);
		break;
	}

	default:
		assert(false);
		break;
	}
	return phi;
}


Constraint ComputePO::sp_assign_helper(Constraint phi, Term * vt, Term * term_E){

	Constraint phi_new = phi;

	//get temp
	Term* temp = VariableTerm::make("sp_binop_temp");

	//v=E[v'/v]
	term_E->replace_term(vt, temp);
	Constraint c_veq(vt, term_E, ATOM_EQ);

	//p[v'/v]
	phi_new.replace_term(vt, temp);

	phi_new = phi_new & c_veq;

	VariableTerm* tempv = static_cast<VariableTerm*>(temp);
	phi_new.eliminate_evar(tempv);

	return phi_new;
}


Constraint ComputePO::sp_assignment(Assignment* assignment, Constraint phi){
	if(DEBUG){
		cout << assignment->to_string() << endl;
	}

	//get old term
	Variable* v = assignment->get_lhs();
	Term* vt;
	if(!v->is_global())
	{
		vt= VariableTerm::make(var_prefix +v->get_var_name());
	}
	else
	{
		vt= VariableTerm::make(v->get_var_name());
	}
	//get new term
	Symbol* sym = assignment->get_rhs();
	Term* new_t = symbol_to_term(sym);


	phi = sp_assign_helper(phi, vt, new_t);

	return phi;
}



Constraint ComputePO::sp_cast(Cast* cast, Constraint phi){
	if(DEBUG)
	{
		cout << cast->to_string() << endl;
	}

	//get old term
	Variable* v = cast->get_lhs();
	Term* vt;
	if(!v->is_global())
	{
		vt = VariableTerm::make(var_prefix +v->get_var_name());
	}
	else
	{
		vt = VariableTerm::make(v->get_var_name());
	}
	//cout << "term vt: " << vt->to_string() << endl;

	//get new term
	Symbol* sym = cast->get_rhs();
	Term* new_t = symbol_to_term(sym);

	//cout << "term new_t: " << new_t->to_string() << endl;

	phi = sp_assign_helper(phi, vt, new_t);

	return phi;
}



Constraint ComputePO::sp_unop(Unop* unop, Constraint phi){
	if(DEBUG){
		cout << unop->to_string() << endl;
	}
	il::unop_type op = unop->get_unop();

	switch (op) {
	case _NEGATE: {
		//can put it before switch
		Variable* v = unop->get_lhs();
		Symbol * sym = unop->get_operand();
		Term* vt;
		if(!v->is_global())
		{
			vt = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			vt = VariableTerm::make(v->get_var_name());
		}
		Term* t1 = symbol_to_term(sym);
		Term* rhs_term = NULL;

		rhs_term = t1->flip_sign();

		//get temp
		Term* temp = VariableTerm::make("sp_unop_temp");
		//cout<< "temp  " << temp->to_string() << endl;

		//x=E[temp/x]
		rhs_term->replace_term(vt, temp);
		Constraint c = Constraint(vt, rhs_term, ATOM_EQ);
		//cout<< "x=E[temp/x] " << c.to_string() << endl;

		//phi[temp/x]
		Constraint phi_temp = phi;
		phi_temp.replace_term(vt, temp);
		//cout<< "phi[temp/x] " << phi_temp.to_string() << endl;

		phi = (c & (phi_temp));

		VariableTerm* tempv = static_cast<VariableTerm*>(temp);
		phi.eliminate_evar(tempv);

		break;
	}
	case _LOGICAL_NOT: /* !x */
	{

		Variable* v = unop->get_lhs();
		Symbol * sym = unop->get_operand();
		Term* vt;
		if(!v->is_global())
		{
			vt = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			vt = VariableTerm::make(v->get_var_name());
		}

		Term* t1 = symbol_to_term(sym);
		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);

		// phi,  \existv' (phi[v'/vt]&vt=term_E[v'/vt])
		Constraint phi0 = sp_assign_helper(phi, vt, t_c0);
		Constraint phi1 = sp_assign_helper(phi, vt, t_c1);

		Constraint c_exp(t1, t_c0, ATOM_NEQ);
		//Constraint c_nexp(t1, t2, ATOM_GEQ);

		//t=!x

		Constraint c_imp_1 = (!c_exp | phi0);// x!=0  implies t = 0;
		Constraint c_imp_2 = (c_exp | phi1); // x=0  implies t = 1;
		phi =  c_imp_1 & c_imp_2;
		break;

	}
	case _PREDECREMENT:
	{
		assert(false);
		break;
	}
	case _PREINCREMENT:  /* --i, ++i */
	{
		assert(false);
		break;
	}
	case _POSTDECREMENT:
	{
		assert(false);
		break;
	}
	case _POSTINCREMENT: /* i--, i++ */
	{
		assert(false);
		break;
	}
	default: {
		//_CONJUGATE, /* complex conjugate */
		//_BITWISE_NOT, /* ~x */
		//_VEC_UNOP

		Variable* v = unop->get_lhs();
		Symbol * sym = unop->get_operand();

		Term* vt;
		if(!v->is_global())
		{
			vt = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			vt = VariableTerm::make(v->get_var_name());
		}
		Term* t1 = symbol_to_term(sym);
		Term* rhs_term = NULL;

		vector<Term*> args;
		args.push_back(t1);

		string name = var_prefix +"fun" + int_to_string(((int) op));
		rhs_term = FunctionTerm::make(name, args, false);

		phi = sp_assign_helper(phi, vt, rhs_term);
		break;
	}
	}
	return phi;
}


Constraint ComputePO::sp_binop(Binop* s_binop, Constraint phi){
	if(DEBUG){
		cout << s_binop->to_string() << endl;
	}
	Variable* v = s_binop->get_lhs();
	Symbol * sym1 = s_binop->get_first_operand();
	Symbol * sym2 = s_binop->get_second_operand();
	Term* vt;
	if(!v->is_global())
	{
		vt = VariableTerm::make(var_prefix +v->get_var_name());
	}else
	{
		vt = VariableTerm::make(v->get_var_name());
	}
	Term* t1 = symbol_to_term(sym1);
	Term* t2 = symbol_to_term(sym2);
	Term* rhs_term = NULL;
	il::binop_type op = s_binop->get_binop();

	switch (op) {
	case _PLUS: {
		//get temp
		Term* temp = VariableTerm::make("sp_binop_temp");

		//v= e[v'/v]
		rhs_term = t1->add(t2);
		rhs_term->replace_term(vt,temp);
		Constraint c = Constraint(vt, rhs_term, ATOM_EQ);


		//phi[temp/x]
		Constraint phi_temp = phi;
		phi_temp.replace_term(vt, temp);
		//cout<< "phi[temp/x] " << phi_temp.to_string() << endl;

		phi = (c & (phi_temp));
		VariableTerm* tempv = static_cast<VariableTerm*>(temp);

		phi.eliminate_evar(tempv);
		break;
	}
	case _MINUS: {
		//get temp
		Term* temp = VariableTerm::make("sp_binop_temp");

		//v= e[v'/v]
		rhs_term = t1->subtract(t2);
		rhs_term->replace_term(vt,temp);
		Constraint c = Constraint(vt, rhs_term, ATOM_EQ);


		//phi[temp/x]
		Constraint phi_temp = phi;
		phi_temp.replace_term(vt, temp);
		//cout<< "phi[temp/x] " << phi_temp.to_string() << endl;

		phi = (c & (phi_temp));
		VariableTerm* tempv = static_cast<VariableTerm*>(temp);

		phi.eliminate_evar(tempv);
		break;
	}
	case _MULTIPLY: {
		//get temp
		Term* temp = VariableTerm::make("sp_binop_temp");

		//v= e[v'/v]
		rhs_term = t1->multiply(t2);
		rhs_term->replace_term(vt,temp);
		Constraint c = Constraint(vt, rhs_term, ATOM_EQ);


		//phi[temp/x]
		Constraint phi_temp = phi;
		phi_temp.replace_term(vt, temp);

		phi = (c & (phi_temp));
		VariableTerm* tempv = static_cast<VariableTerm*>(temp);

		phi.eliminate_evar(tempv);
		break;
	}
	case _MOD: {
		if(sym2->is_constant())
		{
			Constant* c = static_cast<Constant *> (sym2);
			int cons = c->get_integer();
			phi = sp_mod_c(vt, t1, cons, phi);
			break;
		}
		else
		{
			//if op2 is variable
			vector<Term*> args;
			args.push_back(t1);
			args.push_back(t2);
			string name = var_prefix +"fun" + int_to_string(((int) op));
			rhs_term = FunctionTerm::make(name, args, false);
			phi = sp_assign_helper(phi, vt, rhs_term);

			break;

		}
	}
	case _LT: {
		//cout<<"**********EQ*************"<<endl;

		Constraint phi0 = phi;
		Constraint phi1 = phi;

		//get temp
		Term* temp = VariableTerm::make("sp_binop_temp");
		//cout<< "temp  " << temp->to_string() << endl;

		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);

		//v=e[v'/v]
		Constraint c_0(vt, t_c0, ATOM_EQ);
		Constraint c_1(vt, t_c1, ATOM_EQ);

		//p[v'/v]
		phi0.replace_term(vt, temp);
		phi1.replace_term(vt, temp);

		phi0 = phi0 & c_0;
		phi1 = phi1 & c_1;

		VariableTerm* tempv = static_cast<VariableTerm*>(temp);
		phi0.eliminate_evar(tempv);
		phi1.eliminate_evar(tempv);

		Constraint c_exp(t1, t2, ATOM_LT);
		Constraint c_nexp(t1, t2, ATOM_GEQ);

		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi0);
		phi =  c_imp_1 & c_imp_2;
		break;

	}
	case _LEQ: {
		Constraint phi0 = phi;
		Constraint phi1 = phi;

		//get temp
		Term* temp = VariableTerm::make("sp_binop_temp");
		//cout<< "temp  " << temp->to_string() << endl;

		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);

		//v=e[v'/v]
		Constraint c_0(vt, t_c0, ATOM_EQ);
		Constraint c_1(vt, t_c1, ATOM_EQ);

		//p[v'/v]
		phi0.replace_term(vt, temp);
		phi1.replace_term(vt, temp);

		phi0 = phi0 & c_0;
		phi1 = phi1 & c_1;

		VariableTerm* tempv = static_cast<VariableTerm*>(temp);
		phi0.eliminate_evar(tempv);
		phi1.eliminate_evar(tempv);

		Constraint c_exp(t1, t2, ATOM_LEQ);
		Constraint c_nexp(t1, t2, ATOM_GT);

		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi0);
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _GT: {
		//cout<<"**********EQ*************"<<endl;

		Constraint phi0 = phi;
		Constraint phi1 = phi;

		//get temp
		Term* temp = VariableTerm::make("sp_binop_temp");

		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);

		//v=e[v'/v]
		Constraint c_0(vt, t_c0, ATOM_EQ);
		Constraint c_1(vt, t_c1, ATOM_EQ);

		//p[v'/v]
		phi0.replace_term(vt, temp);
		phi1.replace_term(vt, temp);

		phi0 = phi0 & c_0;
		phi1 = phi1 & c_1;

		VariableTerm* tempv = static_cast<VariableTerm*>(temp);
		phi0.eliminate_evar(tempv);
		phi1.eliminate_evar(tempv);

		Constraint c_exp(t1, t2, ATOM_GT);
		Constraint c_nexp(t1, t2, ATOM_LEQ);

		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi0);
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _GEQ: {
		//cout<<"**********EQ*************"<<endl;

		Constraint phi0 = phi;
		Constraint phi1 = phi;

		//get temp
		Term* temp = VariableTerm::make("sp_binop_temp");
		//cout<< "temp  " << temp->to_string() << endl;

		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);

		//v=e[v'/v]
		Constraint c_0(vt, t_c0, ATOM_EQ);
		Constraint c_1(vt, t_c1, ATOM_EQ);

		//p[v'/v]
		phi0.replace_term(vt, temp);
		phi1.replace_term(vt, temp);

		phi0 = phi0 & c_0;
		phi1 = phi1 & c_1;

		VariableTerm* tempv = static_cast<VariableTerm*>(temp);
		phi0.eliminate_evar(tempv);
		phi1.eliminate_evar(tempv);

		Constraint c_exp(t1, t2, ATOM_GEQ);
		Constraint c_nexp(t1, t2, ATOM_LT);

		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi0);
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _EQ: {
		//cout<<"**********EQ*************"<<endl;
		Constraint phi0 = phi;
		Constraint phi1 = phi;

		//get temp
		Term* temp = VariableTerm::make("sp_binop_temp");
		//cout<< "temp  " << temp->to_string() << endl;

		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);

		//v=e[v'/v]
		Constraint c_0(vt, t_c0, ATOM_EQ);
		Constraint c_1(vt, t_c1, ATOM_EQ);

		//p[v'/v]
		phi0.replace_term(vt, temp);
		phi1.replace_term(vt, temp);

		phi0 = phi0 & c_0;
		phi1 = phi1 & c_1;

		VariableTerm* tempv = static_cast<VariableTerm*>(temp);
		phi0.eliminate_evar(tempv);
		phi1.eliminate_evar(tempv);

		Constraint c_exp(t1, t2, ATOM_EQ);
		Constraint c_nexp(t1, t2, ATOM_NEQ);


		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi0);
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _NEQ: {

		Constraint phi0 = phi;
		Constraint phi1 = phi;

		//get temp
		Term* temp = VariableTerm::make("sp_binop_temp");

		Term* t_c0 = ConstantTerm::make(0);
		Term* t_c1 = ConstantTerm::make(1);

		//v=e[v'/v]
		Constraint c_0(vt, t_c0, ATOM_EQ);
		Constraint c_1(vt, t_c1, ATOM_EQ);

		//p[v'/v]
		phi0.replace_term(vt, temp);
		phi1.replace_term(vt, temp);

		phi0 = phi0 & c_0;
		phi1 = phi1 & c_1;

		VariableTerm* tempv = static_cast<VariableTerm*>(temp);
		phi0.eliminate_evar(tempv);
		phi1.eliminate_evar(tempv);

		Constraint c_exp(t1, t2, ATOM_NEQ);
		Constraint c_nexp(t1, t2, ATOM_EQ);

		Constraint c_imp_1 = (c_nexp | phi1);// implies phi1;
		Constraint c_imp_2 = (c_exp | phi0);
		phi =  c_imp_1 & c_imp_2;
		break;
	}
	case _LOGICAL_AND: {
		assert(false);
		break;
	}
	case _LOGICAL_OR: /* &&, || (shortcircuit semantics) */
	{
		assert(false);
		break;
	}
	case _LOGICAL_AND_NO_SHORTCIRCUIT: {
		assert(false);
		break;
	}
	case _LOGICAL_OR_NO_SHORTCIRCUIT: {
		assert(false);
		break;
	}
	default: {
		//DIV and else
		vector<Term*> args;
		args.push_back(t1);
		args.push_back(t2);
		string name = var_prefix +"fun" + int_to_string(((int) op));
		rhs_term = FunctionTerm::make(name, args, false);

		phi = sp_assign_helper(phi, vt, rhs_term);
		break;
	}
	}
	return phi;
}


Constraint ComputePO::sp_assume(Assume* inst_assume, Constraint phi){
	if(DEBUG){
		cout << inst_assume->to_string() << endl;
	}

	Symbol* pred = inst_assume->get_predicate();
	Constraint pred_c;
	if (pred->is_constant()) {
		Constant* c = static_cast<Constant *> (pred);
		if (c->get_integer() == 0) {
			pred_c = Constraint(false);
		} else {
			pred_c = Constraint(true);
		}
	} else {
		Variable * v = static_cast<Variable *> (pred);
		Term* t1;
		if(!v->is_global())
		{
			t1 = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			t1 = VariableTerm::make(v->get_var_name());
		}
		Term* t2 = ConstantTerm::make(0);
		pred_c = Constraint(t1, t2, ATOM_NEQ);
	}
	return pred_c & phi;
}


Constraint ComputePO::sp_assert(StaticAssert* assertion, Constraint phi){
	if(DEBUG){
		cout << assertion->to_string() << endl;
	}
	Symbol* pred = assertion->get_predicate();
	Constraint pred_c;
	if (pred->is_constant()) {
		Constant* c = static_cast<Constant *> (pred);
		if (c->get_integer() == 0) {
			pred_c = Constraint(false);
		} else {
			pred_c = Constraint(true);
		}
	} else {
		Variable * v = static_cast<Variable *> (pred);
		Term* t1;
		if(!v->is_global())
		{
			t1 = VariableTerm::make(var_prefix +v->get_var_name());
		}
		else
		{
			t1 = VariableTerm::make(v->get_var_name());
		}
		Term* t2 = ConstantTerm::make(0);
		pred_c = Constraint(t1, t2, ATOM_NEQ);
	}
	return pred_c & phi;
}


Constraint ComputePO::sp_functionCall(FunctionCall* functionCall, Constraint phi)
{
	if(DEBUG){
		cout << functionCall->to_string() << endl;
	}
	Constraint phi_temp = phi;
	Variable* v = functionCall->get_lhs();


	/*
	 * deal with the loop condition with unknown function
	 */
	string name = functionCall->get_function_name();
	//	string str_unknown = "unknown";
	//	if(strcmp(name.c_str(), str_unknown.c_str()) == 0)
	//	{
	size_t found;
	found = name.find(UNKNOWN_STR);
	size_t found2;
	found2 = name.find(OBSERVED);
	if (found!=string::npos)
	{
		if(v == NULL)
		{
			//lhs of unknown function can never be NULL
			assert(false);
		}
		vector<Term*> args ;

		Term* new_term = VariableTerm::make("__" + name);
		Term* vt = VariableTerm::make(this->var_prefix + v->get_var_name());
		phi = sp_assign_helper(phi, vt, new_term);
		return phi;
	}else if(found2!=string::npos)
	{
		vector<Term*> args;
		Term* rhs_term = FunctionTerm::make(name, args, false);
		Term* vt = VariableTerm::make(this->var_prefix + v->get_var_name());
		phi = sp_assign_helper(phi, vt, rhs_term);
		return phi;
	}else
	{
		/*
		 * I. mapping: caller -->  callee
		 */
		vector<Symbol*>* syms = functionCall->get_arguments();
		vector<Symbol*>::iterator iter;
		vector<Term*> args;
		for(iter = (*syms).begin(); iter != (*syms).end(); iter++)
		{
			args.push_back(symbol_to_term(*iter));
		}
		//string name = functionCall->get_function_name();
		Function * callingfunc = function_finder(name);//, args);


		function_declaration* declare = callingfunc->get_original_declaration();
		string name_declare =  declare->get_name();
		vector<variable_declaration*> args_declare = declare->get_arguments();

		for(unsigned int i = 0; i < args_declare.size(); i++)
		{
			Term *term = VariableTerm::make(name_declare+"::"+ args_declare[i]->get_variable()->get_name());
			if(DEBUG)
			{
				cout << "args :" << args[i]->to_string() << endl;
				cout << "term :" << term->to_string() << endl;
			}
			phi_temp = sp_assign_helper(phi_temp, term, args[i]);
		}

		//cout << "phi_temp : " << phi_temp << endl;

		/*
		 * III. process sub-function
		 */
		CallingContext* cc = new CallingContext(*(cur_eval_key->getcallingcontext()));
		cc->add_context(this->entry_func, functionCall->line);
		POEKey* poekey_func_entry = new POEKey(callingfunc->get_entry_block(), cc);
		POEKey* poekey_func_exit = new POEKey(callingfunc->get_exit_block(), cc);
		if(DEBUG)
		{
			cout << "************************   sub function " << name << " has been called ***********************" << endl;
		}

		Constraint c_after_sp = sp_sub_function(poekey_func_entry, poekey_func_exit, phi_temp, callingfunc);

		if(DEBUG)
		{
			cout << "************************   sub function calling end ***********************" << endl;
		}
		eliminate_temp(c_after_sp);
		if(DEBUG)
		{
			cout << "sp sub function() returns   " << c_after_sp.to_string() << endl;
		}

		phi_temp = c_after_sp;

		/*
		 * IV. return
		 */
		if(v!=NULL)
		{
			//phi = phi&c_after_sp;
			Term* new_t = VariableTerm::make(var_prefix + v->get_var_name());
			Term* old_t = VariableTerm::make(name + "::__return");
			phi_temp = sp_assign_helper(phi_temp, new_t, old_t);
		}else
		{
			//cout << "v==NULL" << endl;
		}

		//elimination
		set<VariableTerm*> vterm_incallee;
		set<Term*> c_terms;
		phi_temp.get_terms(c_terms, false);
		set<Term*>::iterator c_t_iter;

		for(c_t_iter = c_terms.begin(); c_t_iter!= c_terms.end(); c_t_iter++)
		{
			if((*c_t_iter)->type == VARIABLE_TERM){
				VariableTerm* vt = static_cast<VariableTerm*>(*c_t_iter);
				size_t found;
				found = vt->get_name().find(name + "::");
				if (found!=string::npos)
				{
					vterm_incallee.insert(vt);
				}
			}
		}
		//		set<VariableTerm*>::iterator iter_vt;
		//		for(iter_vt = vterm_incallee.begin();iter_vt != vterm_incallee.end(); iter_vt++)
		//		{
		//			cout << "vterm_incallee   :   " << (*iter_vt)->to_string() << endl;
		//		}
		phi_temp.eliminate_evars(vterm_incallee);
		return phi_temp;
		//}
	}
}


Constraint ComputePO::sp_sub_function(POEKey* poekey_func_entry, POEKey* poekey_func_exit, Constraint begin,Function* callingfunc)
{
	ComputePO* newCPO = new ComputePO(callingfunc, this->map_poekey_po, this->func_vector);
	//cout << "poekey_func_entry   :" << poekey_func_entry->to_string() << "poekey_func_exit   : " << poekey_func_exit->to_string() << endl;
	newCPO->compute_func_sp(poekey_func_entry, poekey_func_exit, begin);
	newCPO->get_map_poekey_po(this->map_poekey_po);
	return newCPO->get_extra_after_info();
}



Constraint ComputePO::sp_mod_c(Term* vt, Term* t1, int cons, Constraint phi){
	//if op2 is constant

	Term * term_c_0 = ConstantTerm::make(0);
	Term * term_c_1 = ConstantTerm::make(1);
	Term * term_c_2 = ConstantTerm::make(2);
	Term * term_c_3 = ConstantTerm::make(3);
	Term * term_c_4 = ConstantTerm::make(4);

	Term * term_cons = ConstantTerm::make(cons);

	switch (cons) {
	case 2:
	{

		Constraint phi0 = sp_assign_helper(phi, vt, term_c_0);
		Constraint phi1 = sp_assign_helper(phi, vt, term_c_1);

		Term * t_mod_eq_0 = t1;
		Constraint c_mod_eq_0(t_mod_eq_0, term_cons, ATOM_MOD);

		Term * t_mod_eq_1 = t1->subtract(term_c_1);
		Constraint c_mod_eq_1(t_mod_eq_1, term_cons, ATOM_MOD);

		phi = (!c_mod_eq_0 | phi0);
		phi = phi &(!c_mod_eq_1 | phi1);

		break;
	}
	case 3:
	{
		Constraint phi0 = sp_assign_helper(phi, vt, term_c_0);
		Constraint phi1 = sp_assign_helper(phi, vt, term_c_1);
		Constraint phi2 = sp_assign_helper(phi, vt, term_c_2);


		Term * t_mod_eq_0 = t1;
		Constraint c_mod_eq_0(t_mod_eq_0, term_cons, ATOM_MOD);

		Term * t_mod_eq_1 = t1->subtract(term_c_1);
		Constraint c_mod_eq_1(t_mod_eq_1, term_cons, ATOM_MOD);

		Term * t_mod_eq_2 = t1->subtract(term_c_2);
		Constraint c_mod_eq_2(t_mod_eq_2, term_cons, ATOM_MOD);

		phi = (!c_mod_eq_0 | phi0);
		phi = phi &(!c_mod_eq_1 | phi1);
		phi = phi &(!c_mod_eq_2 | phi2);

		break;
	}
	case 4:
	{
		Constraint phi0 = sp_assign_helper(phi, vt, term_c_0);
		Constraint phi1 = sp_assign_helper(phi, vt, term_c_1);
		Constraint phi2 = sp_assign_helper(phi, vt, term_c_2);
		Constraint phi3 = sp_assign_helper(phi, vt, term_c_3);


		Term * t_mod_eq_0 = t1;
		Constraint c_mod_eq_0(t_mod_eq_0, term_cons, ATOM_MOD);

		Term * t_mod_eq_1 = t1->subtract(term_c_1);
		Constraint c_mod_eq_1(t_mod_eq_1, term_cons, ATOM_MOD);

		Term * t_mod_eq_2 = t1->subtract(term_c_2);
		Constraint c_mod_eq_2(t_mod_eq_2, term_cons, ATOM_MOD);

		Term * t_mod_eq_3 = t1->subtract(term_c_3);
		Constraint c_mod_eq_3(t_mod_eq_3, term_cons, ATOM_MOD);


		phi = (!c_mod_eq_0 | phi0);
		phi = phi &(!c_mod_eq_1 | phi1);
		phi = phi &(!c_mod_eq_2 | phi2);
		phi = phi &(!c_mod_eq_3 | phi3);

		break;
	}
	case 5:
	{
		Constraint phi0 = sp_assign_helper(phi, vt, term_c_0);
		Constraint phi1 = sp_assign_helper(phi, vt, term_c_1);
		Constraint phi2 = sp_assign_helper(phi, vt, term_c_2);
		Constraint phi3 = sp_assign_helper(phi, vt, term_c_3);
		Constraint phi4 = sp_assign_helper(phi, vt, term_c_4);


		Term * t_mod_eq_0 = t1;
		Constraint c_mod_eq_0(t_mod_eq_0, term_cons, ATOM_MOD);

		Term * t_mod_eq_1 = t1->subtract(term_c_1);
		Constraint c_mod_eq_1(t_mod_eq_1, term_cons, ATOM_MOD);

		Term * t_mod_eq_2 = t1->subtract(term_c_2);
		Constraint c_mod_eq_2(t_mod_eq_2, term_cons, ATOM_MOD);

		Term * t_mod_eq_3 = t1->subtract(term_c_3);
		Constraint c_mod_eq_3(t_mod_eq_3, term_cons, ATOM_MOD);

		Term * t_mod_eq_4 = t1->subtract(term_c_4);
		Constraint c_mod_eq_4(t_mod_eq_4, term_cons, ATOM_MOD);


		phi = (!c_mod_eq_0 | phi0);
		phi = phi &(!c_mod_eq_1 | phi1);
		phi = phi &(!c_mod_eq_2 | phi2);
		phi = phi &(!c_mod_eq_3 | phi3);
		phi = phi &(!c_mod_eq_4 | phi4);

		break;
	}
	default: {
		vector<Term*> args;
		args.push_back(t1);
		args.push_back(term_cons);
		string name = "fun_MOD";
		Term* rhs_term = FunctionTerm::make(name, args, false);
		phi = sp_assign_helper(phi, vt, rhs_term);
		break;
	}
	}
	return phi;
}
