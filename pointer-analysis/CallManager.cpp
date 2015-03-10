#include "CallManager.h"
#include "SummaryUnit.h"

#define INTER_PROCEDURE_TEST false

#define CIRCLE_FIND_TEST false

//there are two places, used to chose just one
#define DRAWING_RESOLVED_MEM_GRAPH_AFTER false
int out_put_dot_size = 0;


void CallManager::doing_analysis(const string& output_folder)
{

	//collect all_ids;
	maps_init();

	if (0) {
		cout
				<< "$$$$$$$$$$$$$$ call graph hierarchy testing #################### "
				<< endl;
		print_call_graph_hierarchy();
		//exit(1);
		//print_work_list();
	}

	//analysis all functions without functioncalls,
	//fill up "unresolved_callid_su" data structure and map<call_id, MtdInstsProcessor*>
	if(0){
		cout << "size of unresolved functioncall is " <<
				this->ci_mp.size() << endl;

		exit(1);
	}

	first_round_functions_analysis(output_folder);



	if (0) {

		cout << endl << endl << "TTTTTTTTTTTTesting unresolved functioncall "
				"for each method !!!!!!!!!! " << endl;
		cout << unresolved_fc_for_each_method_to_string();
		exit(1);
	}


	if (0) {
		cout << "TTTTTesting UNREsolved_summary units map "
				"==== ====== ====== ************** " << endl;
		cout << unresolved_summary_unit_to_string() << endl;
		//exit(1);
	}



	interprocedural_analysis(output_folder);


	cout << "TTTTTesting REsolved_summary units map+ "
			"==== ====== ====== ************** " << endl;
	cout << resolved_summary_unit_to_string() << endl;
}


unsigned int CallManager::get_method_idx_from_call_id(
		const call_id& call_id)
{
	assert(ci_mp.count(call_id) > 0);

	MtdInstsProcessor* mdp = ci_mp[call_id];

	assert(mdp != NULL);

	return mdp->get_method_idx();
}



string CallManager::to_string(
		const call_id& call_id)
{

	string str = call_id.to_string();

	str += "  MIDX: [";

	str += int_to_string(get_method_idx_from_call_id(call_id));

	str += "]";

	return str;

}



//to build the summaries of all methods; fill up resolved_callid_su
//and delete all elements from unresolved_callid_su;
void CallManager::interprocedural_analysis(const string& output_folder)
{

	//build up possible resolved_callid_su;
	//include all terminals in app (disconnected - sub_graph)
	cout << "========= Start of interprocedual_analysis ============ " << endl;
	vector<call_id> terminals;
	collect_leaves_in_call_graph(terminals);


	//cout << unresolved_summary_unit_to_string() << endl;

	bool process_circle = false;
	int index = 0;
	while (unresolved_callid_su.size() != 0) {
		//check why always here, and goes to the third branch ?
		index++;

		if(index > 10000){
			cout << unresolved_summary_unit_to_string() << endl;
			exit(1);

		}else{
			cout <<"Unresolved callids size ? " << unresolved_callid_su.size() << endl;
		}

		vector<call_id> newest_terms;

		//used to update its callers and reset terminals as new terms;
		if (terminals.size() > 0) {

			if (1) {
				cout << endl << endl << endl <<
						"----------------------- ROUND-----------------------  " << index << endl;
				cout << "Need to processing [[" << terminals.size() << "]]	Terminals ! " << endl;
				for (int i = 0; i < (int) terminals.size(); i++) {
					cout << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH	["
							<<  i << "]" << endl;

					cout << "Terminal is == >" << terminals.at(i).to_string()
							<< endl;
				}
			}




			terminal_update_callers_and_collect_new_terminals(terminals,
					newest_terms, process_circle,
					output_folder, index);

			cout << "======== Done of update for current terminals !! ======" << endl;

			if (process_circle)
				process_circle = false;

		} else if (worklist.size() > 0) { //no another level of terminals found, try to find circle

			process_circle = true;

			if (0) {
				cout << endl
						<< "ZZZZZZZZZZZZZZZZZZZZZ check worklist for circle finding "
								"ZZZZZZZZZZZZZZZZZ " << endl;
				print_work_list();
			}

			//to find circle, vector holds the order
			vector<call_id> tracked_ids;

			find_one_circle_from_worklist(tracked_ids);

			//have to find circle;
			//如果是circle的，好像还不能直接删除，
			//有可能circle 套circle;仔细考虑，并参看例子！！

			assert(!tracked_ids.empty());

			if (0) {
				cout << "---------------begin of circle id test !------------- "
						<< endl;
				for (int i = 0; i < (int) tracked_ids.size(); i++) {

					cout << "NNNNNNNNNNNNNNNN circle id is "
							<< tracked_ids.at(i).to_string() << endl;
				}

				cout
						<< "------------------ end of circle id test!------------------- "
						<< endl << endl;
			}


			//pre_condition
			//circle_summary_units pre_csu;
			CircleSummaryUnit pre_csu;
			circle_summary_units_init(pre_csu, tracked_ids);



			//process the fix point of the circle;
			call_id succ_ci;
			call_id pred_ci;

			bool fix_point = false;

			int idx = 0;
			map<call_id, call_id> caller_callee;
			while (!fix_point) {
				if(1){
					cout << "Pre summary-unit is 1) ::::::::: " << endl;
					cout << pre_csu.to_string() << endl;
				}


				if(idx  == 5 ){
					cerr << "Check the reason that circle_summary_units "
							"can't reach fix-point! " << endl;

					assert(false);
				}

				cerr << endl <<"(((((((((())))))))))	" << idx << 	"		update elements in circle begin!"<<  endl;
				for (int u = 0; u < (int) tracked_ids.size(); u++) {
					succ_ci = tracked_ids.at(u);

					if ((u + 1) != (int) tracked_ids.size()) //not the last one
						pred_ci = tracked_ids.at(u + 1);

					else
						//the last one
						pred_ci = tracked_ids.at(0);

					update_caller_with_callee(pred_ci, succ_ci, true);
					caller_callee[pred_ci] = succ_ci;
				}
				cerr << "(((((((((())))))))))		" << idx << 	"		update elements in circle end!"
						<< endl << endl<< endl;
				//circle_summary_units post_csu;
				CircleSummaryUnit post_csu;
				circle_summary_units_init(post_csu, tracked_ids);



				if(1){
					cout << "Post CircleSummaryUnit is ::::::::: " << endl;
					cout << post_csu.to_string() << endl;
					cout << "End of Post CircleSummaryUnit !! " << endl;
				}



				if (pre_csu == post_csu) {
					fix_point = true;
					break;
				}
				cout << "calling assignment operator before !" << endl;
				pre_csu = post_csu;
				cout << "calling assignment operator after !" << endl;
				idx ++;
				cout << " EEEEEEnd of this LLLLLLLLoop !! " << endl;
			}

			//从对方的functioncalls 中删除！
			cout << "RACHING FIX POINT FOR CIRCLE! " << endl;

			assert(caller_callee.size() != 0);
			delete_callee_from_caller_for_circle(caller_callee);



			terminals.clear();
			terminals.insert(terminals.begin(), tracked_ids.begin(),
					tracked_ids.end());

			cout << "EEEEEEExit of processing circle summary-unit ! " << endl;

		} else {

			assert(worklist.size() == 0);

		}



	}
	//this one must hold
	assert(unresolved_callid_su.size() == 0);
}


void CallManager::delete_callee_from_caller_for_circle(map<call_id, call_id>&
		caller_callee){

	for(auto it = caller_callee.begin();
			it != caller_callee.end(); it++)
	{
		call_id caller = it->first;
		call_id callee = it->second;
		MtdInstsProcessor* caller_mtdp = ci_mp[caller];
		caller_mtdp->update_unresolved_fc_from_callid(callee);
	}

}


void CallManager::terminal_update_callers_and_collect_new_terminals(
		vector<call_id>& terminals,
		vector<call_id>& newest_terms,
		bool circle_terminals,
		const string& output_folder,
		int& index)
{



	//if it is the terminal that comes from the a circle (in call-graph),
	//circle_ids prepared for checking if callers are part of the circle;
	cout << "for circle_terminals  ? " << circle_terminals << endl;
	set<call_id> circle_ids;
	if (circle_terminals){
		for (int i = 0; i < (int) terminals.size(); i++){
			circle_ids.insert(terminals.at(i));
			cout << "**** inserting circle id is " << terminals.at(i).to_string() << endl;
		}
		cout << endl <<endl;
	}





	for (int m = 0; m < (int) terminals.size(); m++) {

		call_id terminal_id = terminals.at(m);

		//first write them to .dot files, which
		//could be used to check graph right before used to update its callers;

		//when it resolved, draw it out!
		if (!DRAWING_RESOLVED_MEM_GRAPH_AFTER) {

			if (ci_mp[terminal_id]->get_mem_graph() == NULL) {

				cerr << "Terminal "  << terminal_id.to_string() <<
						"	do not contains memgraph ! check why!" << endl;
				assert(ci_mp[terminal_id]->get_mem_graph() != NULL);
			}

			SummaryUnit* su = unresolved_callid_su[terminal_id];
			assert(su != NULL);

			if (1) {
				cout << endl << endl << endl << "	------- " << m << endl;

				cout << ">>>>>>>>>>>>>> drawing to "
						<< su->get_mtd_insts_processor()->get_function()->get_identifier().to_string()
						<< "_resolved! " << endl;
			}


			write_to_dotfiles(
					su->get_mtd_insts_processor()->get_function()->get_identifier(),
					"_resolved! ", output_folder,
					ci_mp[terminal_id]->get_mem_graph());
		}

		//find it in worklist, to update it and delete it from the worklist
		if (worklist.count(terminal_id) > 0) {

			set<call_id>* callers = worklist[terminal_id];

			bool caller_is_circle = false;

			for (set<call_id>::iterator it = callers->begin();
					it != callers->end(); it++) {

				caller_is_circle = false;

				call_id callerid = *it;

				if (1) {
					cout << "              ======  || > processing terminal is "
							<< index++ << "  "  << to_string(terminal_id) << endl;
					cout << "              ====== ||  caller is " <<
							to_string(callerid) << endl;
				}

				//set caller_is_circle flag;
				if (circle_terminals)
					if (circle_ids.count(callerid) > 0)
						caller_is_circle = true;


				/*****
				 * circle has already find the fix-point of their own!
				 * only caller is not in the circle needs to be updated,
				 */
				if (!caller_is_circle)
					update_caller_with_callee(callerid, terminal_id);



				/*
				 * delete resolved leaves from call_graph,
				 * then set current bottom level as "newest_terminals";
				 */
				update_call_graph_hierarchy(callerid, terminal_id, newest_terms,
						caller_is_circle);

			}
		}
		//it is possible that it is leaf but not being called by anyone
		//move them to resolved list
		worklist.erase(terminal_id);

		if (0) {
			cout << " == HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH  == check "
					<< endl;
			cout << "UNRESOLVED SUMs!" << endl
					<< unresolved_summary_unit_to_string() << endl;
			cout << "RRRESOLVED SUMs!" << endl
					<< resolved_summary_unit_to_string() << " end!! " << endl;
		}
	}

//	if(newest_terms.size() == 0)
//		cout << endl << endl<< " PAY ATTENTION!! didn't
// 	" generate new Terminals  !!!!!!!!!!!!!!!!! " << endl;

	//before adding to terminals;
	if(1){
		for(int i = 0; i < (int)newest_terms.size(); i ++){
				cout << "CREATE NEW TERMINAL IS "
						<< newest_terms.at(i).to_string() << endl;
		}
	}

	//after using them to update their callers, we put them to resolved_summaries; and draw the mem_graph
	terminals_to_resolved_map(terminals, circle_terminals, output_folder);


	//reset the element of terminals
	terminals.clear();
	terminals.insert(terminals.begin(),
	newest_terms.begin(), newest_terms.end());
	newest_terms.clear();

	//after adding to terminals;
	if(0){
		for(int i = 0; i < (int)terminals.size(); i ++){
						cout << "CREATE NEW TERMINAL IS ==> " << terminals.at(i).to_string() << endl;
		}
	}
	cout << "EEEEend of terminal_update_callers_and_collect_new_terminals() " << endl;

}



void CallManager::find_one_circle_from_worklist(
		vector<call_id>& circle_ids)
{

	for (map<call_id, set<call_id>*>::iterator it = worklist.begin();
			it != worklist.end(); it++) {

		call_id target_ci = (*it).first;

		bool circle_flag = false;
		find_circle_in_term_of_startpoint(target_ci,
				circle_ids, circle_flag);

		//once find one, then break;
		if(circle_flag)
			break;

		if(CIRCLE_FIND_TEST)
			cout << endl<<"&&&&&&&&&&&&&&&&&&&&&777  CCCCCCClear circle_ids" << endl;
		circle_ids.clear();
	}

}


void CallManager::find_circle_in_term_of_startpoint(const call_id& cur_callid,
		 vector<call_id>& tracked_ids,
		bool& circle_flag)
{

	if (circle_flag == true)
		return;

	assert(worklist.count(cur_callid) > 0);

	set<call_id>* ci_ids = worklist[cur_callid];
	assert(ci_ids != NULL);

	tracked_ids.push_back(cur_callid);

	if (CIRCLE_FIND_TEST)
		cout << endl << "INNNNNNNNNNNNNNNNNNN   " << "     "
				<< cur_callid.to_string() << endl;

	for (set<call_id>::iterator it = ci_ids->begin();
			(it != ci_ids->end()) && (circle_flag == false); it++) {

		call_id pred_id = *it;

		int idx = -1;

		if (CIRCLE_FIND_TEST)
			cout << "         caller (branch) == > " << "     "
					<< pred_id.to_string() << endl << endl;

		if (worklist.count(pred_id) <= 0) { //do nothing
			if (CIRCLE_FIND_TEST)
				cout << "can't find  " << pred_id.to_string() << endl << endl;
		} else if ((idx = find_in_tracked_call_ids(tracked_ids, pred_id))
				!= -1) {
			//the assertion holds because it satisfies the condition of last branch;
			//assert(idx != 0);
			cout << "=========== idx is " << idx << endl;
			//cut elements from the tracked_ids
			tracked_ids.erase(tracked_ids.begin(), tracked_ids.begin() + idx);

			assert(tracked_ids.size() != 0);
			//test the circle;
//			for(int i = 0; i < (int)tracked_ids.size(); i ++){
//
//				cout << "  **** call_id " << tracked_ids.at(i).to_string() << endl;
//			}
			circle_flag = true;
			return;
		} else {

			if (CIRCLE_FIND_TEST)
				cout << endl << "continue searching branch for "
						<< pred_id.to_string() << endl << endl;

			find_circle_in_term_of_startpoint(pred_id, /*start_callid,*/
			tracked_ids, circle_flag);

			//branch didn't find the circle, remove it and
			//find other circle from other branch;
			if (!circle_flag)
				tracked_ids.pop_back();

		}

	}

}


void CallManager::collect_leaves_in_call_graph(vector<call_id>& terminals)
{

	//set<call_id> visited;
	for (unsigned int i = 0; i < funs->size(); i++) {

		Identifier id = funs->at(i)->get_identifier();

		if(0){
			cout << "&&&& " << i << endl;
			cout << "identify is " << id.to_string() << endl;
		}


		call_id ci = identifier_to_call_id(id);

		if(this->is_dummy_id(i)){
			ci = this->get_dummy_id(i);
		}

		//get ride of irreducible method;
		if(this->irreducible_method.count(ci) > 0){
			continue;
		}

		if(call_graph_hierarchy.count(ci) == 0){
			terminals.push_back(ci);
		}


	}

}


/*******
 * current terminal(which has been resolved) move to resolved map;
 */
void CallManager::terminals_to_resolved_map(vector<call_id>& terminals,
		bool circle_t, const string& output_f)
{
	cout << "Size of terminals is " << terminals.size() << endl;
	for (int i = 0; i < (int) terminals.size(); i++) {

		call_id ci = terminals.at(i);

		//if it is circle, already moved from call_graph_hierarchy;
		if (!circle_t)
			assert(call_graph_hierarchy.count(ci) <= 0);

		SummaryUnit* su = unresolved_callid_su[ci];

		//if (INTER_PROCEDURE_TEST)
		cout << "[ " << i << "]" << endl;
		cout << "EEEEErasing " << ci.to_string()
				<< "      from unresolved_callid_su " << endl;

		if(su == NULL){
			cerr << ci.to_string() << "  Need to be erased "
					"	HAS DUPLICATE FUNCTIONS, ALREADY BEEN ERASED, PLEASE CHECK!!" << endl;
			//assert(false);
			continue;
		}

		resolved_callid_su[ci] = su;

		unresolved_callid_su.erase(ci);

		assert(resolved_callid_su[ci] != NULL);


		if (DRAWING_RESOLVED_MEM_GRAPH_AFTER) {
			assert(ci_mp[ci]->get_mem_graph() != NULL);

			write_to_dotfiles(
					su->get_mtd_insts_processor()->get_function()->get_identifier(),
					"_resolved! ", output_f, ci_mp[ci]->get_mem_graph());
		}


	}
	cout << "Erasing all terminals this round ! " << endl;
}


void CallManager::update_call_graph_hierarchy(const call_id& caller_id,
		const call_id& callee_id, vector<call_id>& newest_terms,
		bool caller_in_circle)
{

	assert(call_graph_hierarchy.count(caller_id) > 0);
	if (call_graph_hierarchy.count(caller_id) > 0) {

		call_graph_hierarchy[caller_id]->erase(callee_id);

		if (call_graph_hierarchy[caller_id]->size() == 0) {
				call_graph_hierarchy.erase(caller_id);

				if(!caller_in_circle)
					newest_terms.push_back(caller_id);
		}

	}

}


void CallManager::collect_all_ids()
{

	for (unsigned int i = 0; i < funs->size(); i++) {
		sail::Function* f = funs->at(i);

		//for the purpose of skip irreducible function, which contains INVALID TYPE
//		if(f->is_empty())
//			continue;

		Identifier id = f->get_identifier();
		call_id ci = identifier_to_call_id(id);
		if(call_ids.count(ci) == 0){
			call_ids.insert(ci);

			if(f->is_empty()){
				irreducible_method.insert(ci);
			}
		}else{ //what is already have ? rename the functions and data-structures;

			int index = 0;
			if (duplicated_ids.count(ci) == 0) {
				index = 1;
			}else{
				index = duplicated_ids[ci];
				++ index;
			}

			//make dummy call_id;
			call_id cid(id.get_function_name() + "__" + int_to_string(index),
					id.get_namespace(),
					id.get_function_signature(),
					id.get_language());

			call_ids.insert(cid);
			//keep track the dummy call_ids;
			duplicated_ids[ci] = index;
			dummy_duplicated_ids[ci].insert(cid);

			//prepare for cid->mtdinstsprocess map
			dummy_ids[i] = cid;

			if(f->is_empty())
				irreducible_method.insert(cid);
		}
	}
}


void CallManager::collect_call_graph_hierarchy()
{

	for (unsigned int i = 0; i < funs->size(); i++) {

		sail::Function* f = funs->at(i);

		//for the purpose of skip irreducible methods;
		if(f->is_empty())
			continue;

		//call_id of it self
		Identifier id = f->get_identifier();

		call_id self_ci = identifier_to_call_id(id);

		if(is_dummy_id(i)){ //for the purpose that caller is dummy_id;
			self_ci = this->get_dummy_id(i);
		}


		for (vector<sail::Instruction*>::iterator it = f->get_body()->begin();
				it != f->get_body()->end(); it++) {

			sail::Instruction* cur_ins = *it;

			if (cur_ins->get_instruction_id() == sail::FUNCTION_CALL) {

				sail::FunctionCall* fc =
						static_cast<sail::FunctionCall*>(cur_ins);

				//consider if LIB_HAS_EXCEPTION is off, then should test the condition;
				if (fc->is_reachable()) {


					call_id ci(fc->get_function_name(), fc->get_namespace(),
							fc->get_signature(), lang_type::LANG_JAVA);

					set<call_id> targets;
					targets.insert(ci);


					//if fc's call_id is shared by multiple methods;
					if(dummy_duplicated_ids.count(ci) > 0){

						set<call_id>& same_ids = dummy_duplicated_ids[ci];
						for(auto it = same_ids.begin();
								it != same_ids.end(); it++){
							targets.insert(*it);
						}

					}


					//not virtual if constructor;
					find_virtual_targets(ci, fc, targets);

					//add to call_graph hierarchy
					for (set<call_id>::iterator it = targets.begin();
							it != targets.end(); it++) {

						call_id cur_ci = *it;

						//for the purpose of skip the relation of irreducible_methods,
						//which contains INVALID types, not suitable for analysis;
						if(irreducible_method.count(cur_ci) > 0){
							continue;
						}

						//if is-self-defined functions;
						if (call_ids.count(cur_ci) > 0) {

							if (call_graph_hierarchy.count(self_ci) <= 0) {
								set<call_id>* civ = new set<call_id>();
								civ->insert(cur_ci);
								call_graph_hierarchy[self_ci] = civ;

							} else
								call_graph_hierarchy[self_ci]->insert(cur_ci);

							//add to worklist
							if (worklist.count(cur_ci) <= 0) {
								set<call_id>* civ = new set<call_id>();
								civ->insert(self_ci);
								worklist[cur_ci] = civ;

							} else {
								worklist[cur_ci]->insert(self_ci);
							}

						}

					}

				}
			}
		}
	}
}


string CallManager::all_ids_to_string()const
{
	string str = "All callids collected ::::::::: ";
	str += "\n";
	str += "Total size is ";
	str += int_to_string(call_ids.size());
	str += "\n";
	auto it = this->call_ids.begin();
	int i = 0;
	for(; it != this->call_ids.end(); it++){
		str += "\n";
		str += "[";
		str += int_to_string(i++);
		str +=	"]  ---- ";
		str += (*it).to_string();
	}
	return str;
}



string CallManager::call_graph_hierarchy_to_string() const
{
	string str = "SIZE :: ";
	str += int_to_string(call_graph_hierarchy.size());

	for(map<call_id, set<call_id>*>::const_iterator it = call_graph_hierarchy.begin();
			it != call_graph_hierarchy.end(); it ++){
		str += "\n";
		str += "Call_Id ";
		str += (it->first).to_string();

		str += "   Callees : ";

		int i  = 0;
		for(set<call_id>::iterator it1 = it->second->begin();
				it1 != it->second->end(); it1++){
			str +="\n";
			str += int_to_string(++i);
			str += "): ";

			str += (*it1).to_string();
		}
		str += "\n";
	}

	return str;

}

string CallManager::worklist_to_string() const
{
	string str = "SIZE :: ";
	str += int_to_string(worklist.size());

		for(map<call_id, set<call_id>*>::const_iterator it = worklist.begin();
				it != worklist.end(); it ++){
			str += "\n";
			str += "Callee ID ::";
			str += (it->first).to_string();

			str += "   Callers : ";
			int i  = 0;
			for(set<call_id>::iterator it1 = it->second->begin();
					it1 != it->second->end(); it1++){
				str +="\n";
				str += int_to_string(++i);
				str += "): ";

				str += (*it1).to_string();
			}
			str += "\n";
		}

		return str;
}

void CallManager::collect_call_id_summary_unit(const call_id& call_id,
		SummaryUnit* su)
{

	if (unresolved_callid_su.count(call_id))
		return;

	assert(su != NULL);
	unresolved_callid_su[call_id] = su;
	//cerr << endl <<"?????????? build summarization for " << call_id.to_string() << endl;
}

bool CallManager::is_dummy_id(int f_idx)
{

	return (dummy_ids.count(f_idx) > 0);
}

call_id& CallManager::get_dummy_id(int f_idx)
{
	return dummy_ids[f_idx];
}


bool CallManager::is_dummy_callid(call_id& callid)
{
	return (this->dummy_duplicated_ids.count(callid) > 0 );
}


set<call_id>& CallManager::get_dummy_duplicated_ids(call_id& callid)
{

	return dummy_duplicated_ids[callid];

}


bool CallManager::inside_collect_call_ids(call_id & call_id) const
{

	return (call_ids.count(call_id) > 0);

}

void CallManager::collect_ci_mp(const call_id& ci,
			MtdInstsProcessor* mtdp)
{
	ci_mp[ci] = mtdp;
}



void CallManager::update_caller_with_callee(
		const call_id& callerid,
		const call_id& calleeid, const bool inside_circle)
{

	//被调函数fun-signature
	il::function_type* callee_fs = (il::function_type*)(calleeid.get_signature());
	vector<il::type*> parameter_ts = callee_fs->arg_types;


	//e.g to find the mtdp of a;
	assert(ci_mp.count(callerid) > 0 );
	MtdInstsProcessor* caller_mtdp = ci_mp[callerid];


	//process callee's summarization
	assert(unresolved_callid_su.count(calleeid) > 0);
	SummaryUnit* callee_su = unresolved_callid_su[calleeid];
	assert(callee_su != NULL);

	if (1) {
		cout << "UUUUUUpdate_caller_with_callee()  " << endl;
		cout << endl << endl << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "
				<< out_put_dot_size ++ << endl;
		cout << "calleeid is " << calleeid.to_string() << endl << "SUM :: "
				<< callee_su->to_string()
				<< " ---------------------------------------------------||"
				<< endl << endl;

		cout << "callerid is " << callerid.to_string() << endl << "SUM :: "
				<< caller_mtdp->get_summary_unit()->to_string()
				<< "------------------------------------------------------- ||"
				<< endl << endl;

		cerr <<endl << endl << endl << "calleeid is " << calleeid.to_string() << endl;

		cerr << "callerid is " << callerid.to_string() << endl;

	}



	vector<sail::FunctionCall*>* fcs =
			caller_mtdp->get_unresolved_fc_from_callid(calleeid);


	for(vector<sail::FunctionCall*>::iterator it = fcs->begin();
			it != fcs->end(); it ++){

		sail::FunctionCall* fc = *it;

		vector<sail::Symbol*>* args = fc->get_arguments();
		assert(args->size() == parameter_ts.size());

		cout << "Callee's function signature is :: " << callee_fs->to_string() << endl;
		//实参
		il::type* arg_t = NULL;
		il::type* para_t = NULL;
		bool need_to_process = true;

		for(int i = 0; i != (int)args->size(); i ++){
			arg_t = args->at(i)->get_type();
			para_t = parameter_ts.at(i);
			if(compatible_for_function_call(arg_t, para_t))
				continue;
			else{
				need_to_process = false;
				break;
			}
		}

		if(!need_to_process)
			continue;

		cout << "Functioncall is ::::: " << fc->to_string()
				<<"		Need to process the function call!!!!!!  " << endl;

		//check if the function call makes sense; (argument is base type of declared function signature)

		bool updated = false;
		//this one will add new instructions into the caller
		//(caller_mtdp used to refer to caller);
		caller_mtdp->apply_summary_unit_to_func(fc, callee_su, updated);
		cout << endl <<"hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh " << endl << endl;
//		if(updated)
//			return;
		//process collect_init now;
	}


	//find a new fix point for the caller itself
	//cout << "inside update_caller_with_callee & calling process_instruction_in_rule() " << endl;
	caller_mtdp->process_instructions_in_rule();

	if (0) { //only for test, comment out it later;
		cout << "Writing for =========== >> " << out_put_dot_size << endl;
		write_to_dotfiles(
				caller_mtdp->get_function()->get_identifier(), "",
				"/scratch/mem_graph/", caller_mtdp->get_mem_graph());
	}

	caller_mtdp->reset_summary_unit();
	//store it to unsolved_id_sum map;
	unresolved_callid_su[callerid] = caller_mtdp->get_summary_unit();

	//delete it from unresolved_fc map (if inside_circle, then do it after fix-point);
	if(!inside_circle)
		caller_mtdp->update_unresolved_fc_from_callid(calleeid);

	if(1)
		cout << " Post SUM :: " << caller_mtdp->get_summary_unit()->to_string() << endl;
		//what if another summary_unit will effect the
}




void CallManager::circle_summary_units_init(CircleSummaryUnit& csu,
		vector<call_id>& circle_ids)
{

	for (int k = 0; k < (int) circle_ids.size(); k++) {

		call_id cur_ci = circle_ids.at(k);
		assert(unresolved_callid_su.count(cur_ci) > 0);
		SummaryUnit* su = unresolved_callid_su[cur_ci];

		if (1) {
			cout << "call_id " << cur_ci.to_string() << "inside circle !"
					<< endl;
			cout << " whose  ((()))   content is " << su->to_string() << endl;
		}
		csu.add_summary_unit(su);

	}
}


void CallManager::maps_init()
{
	collect_all_ids();
	//call_graph_hierarchy and worklist map
	cout << duplicated_dummy_ids_to_string() << endl;
	cout << this->dummy_ids_to_string() << endl;

	collect_call_graph_hierarchy();
}



void CallManager::print_call_graph_hierarchy() const
{
	map<call_id, set<call_id>*>::const_iterator it = call_graph_hierarchy.begin();
	int i = 0;
	for(; it != call_graph_hierarchy.end(); it ++){
		cout << "Caller : [" <<i <<"] " << it->first.to_string() <<endl;
		set<call_id>* callees = it->second;

		for(set<call_id>::iterator it_callees = callees->begin();
				it_callees != callees->end(); it_callees++){
			cout << "   === > callees :" << (*it_callees).to_string() << endl;
		}
		cout << endl << endl;
		i++;
	}
	cout << endl;

}


void CallManager::print_work_list() const
{
	map<call_id, set<call_id>*>::const_iterator it = worklist.begin();
		int i = 0;
		for(; it != worklist.end(); it ++){
			cout << "Callee : " << it->first.to_string() << "  ["<< i << "]" <<endl;
			set<call_id>* callers = it->second;

			for(set<call_id>::iterator it_callers = callers->begin();
					it_callers != callers->end(); it_callers++){
				cout << "   === > callers :" << (*it_callers).to_string() << endl;
			}
			cout << endl << endl;
			i++;
		}
		cout << endl;

}

void CallManager::first_round_functions_analysis(const string& output_folder)
{
	cout << "!!! first_round_functions_analysis start !!! " << endl;
	cout << "Total number of method needs to process " << funs->size() << endl
			<< get_time() << endl;

	for(unsigned int i = 0 ; i < funs->size(); i ++){

			if(1)
				cout<<endl << endl  <<
					"	method start(first round function analysis) "
					"*************** == = = = = = = = >>>> "
					<< i << endl
					<< get_time() << endl;


			first_round_fun_analysis(i, output_folder);
		}
}



void CallManager::first_round_fun_analysis(const unsigned int& i,
		const string& output_f)
{

	MtdInstsProcessor* mp =
			new MtdInstsProcessor(i, *funs, output_f); //one per method

	if (MtdInstsProcessor::get_call_manager() == NULL)
		mp->set_call_manager(this);


	//PAY ATTENTION;
	mp->process_basic_instructions();

	if (0) {
		if (i == 923) {
			auto it = mp->get_unresolved_fcs().begin();
			int k = 0;
			for (; it != mp->get_unresolved_fcs().end(); it++) {
				cout << "calleeid is " << k ++ <<
						"  " <<  it->first.to_string() << endl;

				auto iter = it->second->begin();
				int i = 0;
				for (; iter != it->second->end(); iter++) {
					cout << i++ << ")	Corresponding functioncall is "
							<< (*iter)->to_string() << endl;
				}
				cout << endl;
			}
			exit(1);
		}

	}
}





const string CallManager::unresolved_summary_unit_to_string() const
{
	string str = "Unresolved summaryunit size : ";
	str += int_to_string(unresolved_callid_su.size());
	str += "\n";
	string temp = str;
	cout << temp << endl;
	for (map<call_id, SummaryUnit*>::const_iterator it =
			unresolved_callid_su.begin();
			it != unresolved_callid_su.end(); it++) {
		str += "unresolved_callid_su ######################### ";
		str += "\n";
		str += (*it).first.to_string();

		str += "\n";
		if(it->second == NULL){
			cerr << it->first.to_string() << "	summary-unit is NULL ";
			assert(false);
		}
		str += (*it).second->to_string();

		str += "\n";
	}

	return str;
}


const string CallManager::unresolved_fc_for_each_method_to_string()const
{

	string str;
	map<call_id, MtdInstsProcessor*>::const_iterator it = ci_mp.begin();
	int i = 0;
	for(; it != ci_mp.end(); it++){
		str += "[";
		str += int_to_string(i++);
		str += "]  ";
		str += it->first.to_string();
		str += "\n";
		str += "UUUUnresolved functioncalls are : ";
		str += "\n";
		str += it->second->unresolved_functioncall_to_string();
		str += "\n";
		str += "\n";
	}

	return str;
}

const string CallManager::resolved_summary_unit_to_string() const
{
	string str;

	for (map<call_id, SummaryUnit*>::const_iterator it =
			resolved_callid_su.begin(); it != resolved_callid_su.end();
			it++) {
		str += "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  ";
		str += "\n";
		str += (*it).first.to_string();

		str += "\n";

		str += (*it).second->to_string();

		str += "\n";
	}

	return str;
}

const string CallManager::duplicated_dummy_ids_to_string() const
{
	string str;
	auto it = dummy_duplicated_ids.begin();

	for(; it != dummy_duplicated_ids.end(); it++){
		str+= "Derive from : ";
		str += it->first.to_string();
		str += "\n";
		for(auto iter = it->second.begin(); iter != it->second.end();
				iter++){
			str += "	As :";
			str += (*iter).to_string();
		}
		str += "\n";
	}
	return str;
}


const string CallManager::dummy_ids_to_string()const
{

	string str;

	auto it = this->dummy_ids.begin();

	for(; it != this->dummy_ids.end(); it++){
		str += "Mtd Index ";
		str += int_to_string(it->first);
		str += "\n";
		str += it->second.to_string() ;
	}
	return str;
}


void CallManager::generate_different_allocsite_id(
			int line_number, int instruction_num,
			Identifier& id, alloc_info& ai)
{

		info_item ii(line_number, instruction_num,
						id);

		cout << "~~~~~**** 1) info_item is " << ii.to_string() << endl;

		bool flag = false;
		assert(this != NULL);
		while(this->alloc_site.count(ii) > 0)
		{
			flag = true;
			line_number ++;
			instruction_num ++;

			//update ii;
			ii = info_item(line_number, instruction_num, id);
		}

		if(flag)
			cout << "~~~~~**** 2) info_item is " << ii.to_string() << endl;
		this->alloc_site.insert(ii);

		ai = alloc_info(line_number, instruction_num,
				id);
}
