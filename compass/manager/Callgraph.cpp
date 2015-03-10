/*
 * Callgraph.cpp
 *
 *  Created on: Aug 20, 2008
 *      Author: tdillig
 */

#include "Callgraph.h"
#include "sail/Function.h"
#include "sail/Instruction.h"
#include "sail/FunctionPointerCall.h"
#include "sail/FunctionCall.h"
#include "sail/Block.h"
#include "sail/BasicBlock.h"
#include "sail/SuperBlock.h"
#include "sail/CfgEdge.h"
#include "sail/Cfg.h"
#include "sail/Variable.h"
#include "sail/AddressLabel.h"
#include <assert.h>
#include <iostream>
#include "util.h"
#include "il/type.h"
#include <string.h>
#include "Error.h"
#include "ClientAnalysis.h"
#include "TaintAnalysis.h"

#define DEBUG true

using namespace sail;

/*
 * The set of functions to ignore in the callgraph.
 * This should include all functions defined in
 * runtime/compass-runtime.h. Terminate with empty string.
 */
string functions_to_ignore[] =
{
		"static_assert",
		"check_buffer",
		"check_null",
		"check_uninit",
		"assume",
		"static_choice",
		"check_cast",
		"check_memory_leak",
		"check_double_delete",
		"check_deleted_access",
		"observed",
		""

};

set<string> Callgraph::read_fields;
set<string> Callgraph::written_fields;


cg_edge_id::cg_edge_id(bool definite_target)
{
	id = 0;
	if(definite_target) {
		id |= (1 << 1);
	}
}

cg_edge_id::cg_edge_id()
{
	id = 0;
}

bool cg_edge_id::is_definite() const
{
	return (id & (1<<1));
}

bool cg_edge_id::is_backedge() const
{
	return id & 1;
}

void cg_edge_id::mark_backedge()
{
	id |= 1;
}

CGNode::CGNode(const Identifier& su_id){
	cg_id = -1;
	id = su_id;

}
CGNode::CGNode(const Identifier & su_id, int id){
	cg_id = id;
	this->id = su_id;

}




string CGNode::get_serial_id()
{
	return id.to_string('#');
}


string CGNode::get_function_serial_id()
{
	return id.get_function_identifier().to_string('#');
}



bool CGNode::is_function()
{
	return id.is_function_id();
}

void CGNode::add_callee(CGNode* callee, bool definite_target){
	callees[callee] = cg_edge_id(definite_target);
	callee->callers[this] = cg_edge_id(definite_target);

}

bool CGNode::has_callers(){
	return callers.size()!=0;
}

bool CGNode::has_callees(){
	return callees.size()!=0;
}


const map<CGNode*, cg_edge_id>& CGNode::get_callees()
{
	return callees;
}
const map<CGNode*, cg_edge_id>& CGNode::get_callers()
{
	return callers;
}





bool CGNodeLessThan::operator()(const CGNode* n1, const CGNode* n2) const
{
	return n1->cg_id < n2->cg_id;
}




CGNode* Callgraph::get_checks_fn()
{
	return this->checks;
}

set<CGNode*>& Callgraph::get_init_functions()
{
	return this->init_functions;
}

void Callgraph::get_transitive_callees(CGNode* start,
		set<CGNode*>& transitive_callees)
{
	set<CGNode*> worklist;
	worklist.insert(start);

	while(worklist.size() > 0)
	{
		CGNode* cur = *worklist.begin();
		worklist.erase(cur);
		if(transitive_callees.count(cur) > 0) continue;
		transitive_callees.insert(cur);
		const map<CGNode*, cg_edge_id>& callees = cur->get_callees();
		map<CGNode*, cg_edge_id>::const_iterator it = callees.begin();
		for(; it!= callees.end(); it++)
		{
			cg_edge_id ei = it->second;
			if(ei.is_backedge()) continue;
			worklist.insert(it->first);
		}
	}

}

/*
 * Gives the set of CGNode's transitively reachable *only*
 * from "start".
 */
void Callgraph::get_exclusive_callees(CGNode* start, set<CGNode*>&
		exclusive_callees)
{
	map<Identifier, CGNode*>::iterator it = nodes.begin();
	for(; it!= nodes.end(); it++)
	{
		CGNode* cur = it->second;
		assert(dominators.count(cur) > 0);
		set<CGNode*> & doms = dominators[cur];
		if(doms.count(start) > 0) exclusive_callees.insert(cur);
	}
}

const map<call_id, Identifier>& Callgraph::get_stubs()
{
	return this->orig_to_spec;
}


void Callgraph::process_adt_functions(sail::Function* f)
{
	const Identifier & id = f->get_identifier();
	string name = id.get_function_name();

	il::type_attribute_kind att = il::NO_TYPE_ATTRIBUTE;
	if(name.find(POSITION_DEPENDENT_ADT_PREFIX)!=string::npos)
	{
		att = il::POSITION_DEPENDENT_ADT;
	}
	else if(name.find(SINGLE_VALUED_ADT_PREFIX)!=string::npos)
	{
		att = il::SINGLE_VALUED_ADT;
	}
	else if(name.find(MULTI_VALUED_ADT_PREFIX)!=string::npos)
	{
		att = il::MULTI_VALUED_ADT;
	}

	if(att != il::NO_TYPE_ATTRIBUTE)
	{
		il::function_type* ft = f->get_signature();
		assert(ft->get_arg_types().size() >= 1);
		il::type* arg_t = ft->get_arg_types()[0];
		arg_t = arg_t->get_inner_type();


		il::type* key_t = il::get_void_ptr_type();
		il::type* val_t = il::get_void_ptr_type();
		if(ft->get_arg_types().size() == 3)
		{
			key_t = ft->get_arg_types()[1]->get_inner_type();
			val_t = ft->get_arg_types()[2]->get_inner_type();
			assert(key_t != NULL);
			assert(val_t != NULL);

		}

		assert(arg_t != NULL);
		arg_t->set_attribute(att, key_t, val_t);
	}




	/*
	if(name.find(POSITION_DEPENDENT_ADT_PREFIX)!=string::npos)
	{
		cout << "pos ADT!!!!! " << endl;
		il::function_type* ft = f->get_signature();
		assert(ft->get_arg_types().size() >= 1);
		il::type* arg_t = ft->get_arg_types()[0];
		arg_t = arg_t->get_inner_type();
		cout << "SIG:" << id.get_function_signature()->to_string() << endl;
		cout << "Arg t: " << arg_t->to_string() << endl;
		assert(arg_t != NULL);
		arg_t->set_attribute(il::POSITION_DEPENDENT_ADT);
	}
	else if(name.find(SINGLE_VALUED_ADT_PREFIX)!=string::npos)
	{
		il::function_type* ft = f->get_signature();
		assert(ft->get_arg_types().size() == 1);
		il::type* arg_t = ft->get_arg_types()[0];
		arg_t = arg_t->get_inner_type();
		assert(arg_t != NULL);
		arg_t->set_attribute(il::SINGLE_VALUED_ADT);
	}
	else if(name.find(MULTI_VALUED_ADT_PREFIX)!=string::npos)
	{
		il::function_type* ft = f->get_signature();
		assert(ft->get_arg_types().size() == 1);
		il::type* arg_t = ft->get_arg_types()[0];
		arg_t = arg_t->get_inner_type();
		assert(arg_t != NULL);
		arg_t->set_attribute(il::MULTI_VALUED_ADT);
	}*/




	if(name.find(COMPASS_SPEC_PREFIX) != string::npos)
	{
		int pos = name.find(COMPASS_SPEC_PREFIX);
		int start = pos + strlen(COMPASS_SPEC_PREFIX);
		string orig_name = name.substr(start);
		Identifier orig_id(f->get_identifier().get_file_id(), orig_name,
				f->get_namespace(),
				f->get_identifier().get_function_signature(),
				f->get_identifier().get_language());
		orig_to_spec[call_id(orig_id)] = f->get_identifier();
		ids_with_spec.insert(orig_id);


	}
}

bool Callgraph::track_rtti()
{
	return track_dynamic_types;
}


void Callgraph::map_call_ids_to_identifiers(set<string>& sail_serial_ids)
{
	if(DEBUG){
		cout << "************ Mapping call ids to indentifiers **************"
				<< endl;
	}
	set<string>::iterator it = sail_serial_ids.begin();
	for(; it!= sail_serial_ids.end(); it++)
	{
		const string& cur_id = *it;
		sail::Function* f= (sail::Function*) dm->get_data(cur_id, SAIL_FUNCTION);
		assert(f!= NULL);
		process_adt_functions(f);
		Identifier id = f->get_identifier();
		string file = id.get_file_id().to_string();
		call_id ci(id.get_function_name(), id.get_namespace(),
				id.get_function_signature(), id.get_language());


		if(id.get_language() == LANG_CPP || id.get_language() == LANG_JAVA) {
			track_dynamic_types = true;
		}


		if(id.get_function_name().find("static_choice")!=string::npos) continue;

		if(call_id_to_identifier.count(ci) > 0) {
			/*
			cout << "ALREADY IN SET" << endl;
			map<call_id, Identifier>::iterator d_it =
					call_id_to_identifier.begin();
			for(; d_it != call_id_to_identifier.end(); d_it++)
			{
				cout << "\t IN MAP: " << d_it->first.to_string() << endl;
			}
			*/

			Error* e = new Error(f->get_first_line(), file,
					ERROR_UNRESOLVED_CALL_ID, " Another function with "
							"matching name and signature is defined in " +
							call_id_to_identifier[ci].get_file_id().to_string(),
							Identifier());
			errors[file].insert(e);
		}
		else {
			call_id_to_identifier[ci] = id;
		}

		if(DEBUG){
			cout << "Call id: " << id.to_string() << " Indentifier: " <<
					id.to_string() << endl;
			cout << "name: " << id.get_function_name() << endl;
			cout << "ns: " << id.get_namespace().to_string() << endl;
			cout << "sig: " << id.get_function_signature()->to_string() << endl;
			cout << "lang: " << id.get_language() << endl;
		}



		dm->mark_unused(cur_id, SAIL_FUNCTION);
	}
	if(DEBUG){
		cout << "**************************************" << endl;
	}

}

void Callgraph::remove_node(CGNode* node)
{

	nodes.erase(node->id);
	entry_points.erase(node);
	init_functions.erase(node);
	dominators.erase(node);

	const map<CGNode*, cg_edge_id>& callers = node->get_callers();
	map<CGNode*, cg_edge_id>::const_iterator it = callers.begin();
	for(; it!= callers.end(); it++) {
		CGNode* caller = it->first;
		caller->callees.erase(node);
	}

	const map<CGNode*, cg_edge_id>& callees = node->get_callees();
	it = callees.begin();
	for(; it!= callees.end(); it++) {
		CGNode* callee = it->first;
		callee->callers.erase(node);
	}
	delete node;
}

void Callgraph::process_instruction(sail::Instruction* inst)
{

	if(!COLLECT_FIELDS) return;
	if(inst->get_instruction_id() == LOAD)
	{

		sail::Load* l = static_cast<sail::Load*>(inst);
		string name = l->get_field_name();
		if(name != "") {
			read_fields.insert(name);
		}
	}
	if(inst->get_instruction_id() == STORE)
	{

		sail::Store* s = static_cast<sail::Store*>(inst);
		string name = s->get_field_name();
		if(name != "") {
			written_fields.insert(name);
		}
	}
}

bool Callgraph::field_written(const string & s)
{
	return written_fields.count(s) > 0;
}

bool Callgraph::field_read(const string & s)
{
	return read_fields.count(s) > 0;
}


void Callgraph::process_summary_unit(sail::SummaryUnit* su,
		bool process_function_address)
{

	CGNode* cur_node = get_node(su->get_identifier());




	language_type cur_lang = su->get_identifier().get_language();

	if(su->is_function()) {
		sail::Function* f = (sail::Function*) su;
		if(f->is_init_function()) {
			init_functions.insert(cur_node);
		}
	}

	BasicBlock* b = su->get_entry_block();
	BasicBlock* exit_b = su->get_exit_block();
	set<Block*> worklist;
	set<Block*> visited;
	worklist.insert(b);

	if(DEBUG)
	{
		cout << "######## processing " << su->get_identifier().to_string() << " ###### "
				<< endl;
	}


	while(worklist.size() > 0)
	{
		Block* b = *worklist.begin();
		worklist.erase(b);
		if(visited.count(b) > 0) continue;
		visited.insert(b);

		//cout << "Processing block: " << b->get_block_id() << endl;
		if(b->is_superblock()) {
			SuperBlock* sb = (SuperBlock*) b;
			CGNode* callee_node = get_node(sb->get_identifier());
			cur_node->add_callee(callee_node, true);
			process_summary_unit(sb, process_function_address);

		}

		else {
			BasicBlock* bb = (BasicBlock*) b;
			vector<Instruction*>& instructions = bb->get_statements();
			for(int i=0; i<(int)instructions.size(); i++){
				Instruction* cur = instructions[i];
				process_instruction(cur);
				instruction_type inst_type = cur->get_instruction_id();
				if(!process_function_address && inst_type==FUNCTION_CALL){

					FunctionCall* fc = (FunctionCall*) cur;
					cout << "processing function call " << fc->to_string(false)
							<< endl;
					//cout << "virtual call? " << fc->is_virtual_call() << endl;
					cout << "constructor? " << fc->is_constructor() << endl;
					if(fc->is_super())
					{
						cout << "call to super function" << endl;
						//Call to the super function.
						assert(fc->get_arguments()->size() > 0);
						il::type* t = (*fc->get_arguments())[0]->get_type();
						assert(t->is_pointer_type());
						t = t->get_deref_type();
						assert(t->is_record_type());
						il::record_type* rt = static_cast<il::record_type*>(t);
						il::method_info* mi = NULL;
						while(true)
						{

							cout << "cur rt: " << rt->to_string() << endl;
							mi = fc->find_matching_method(rt,
									fc->get_function_name(),
										fc->get_signature());
							if(mi != NULL) break;
							const map<int, il::record_type*> & bases = rt->get_bases();
							auto it = bases.find(0);
							if(it == bases.end()) break;
							rt = it->second;
						}
						/*

						cout << "type: " << rt->to_string() << endl;
						auto map = rt->get_bases();

						il::method_info* mi = NULL;
						if(map.find(0) != map.end())
						{
							//assert(map.find(0) != map.end());
							il::record_type* super = map.find(0)->second;
							mi = fc->find_matching_method(super,
									fc->get_function_name(),
										fc->get_signature());
						}
						*/

						if(mi != NULL)
						{
							call_id cid(mi->name, mi->ns, mi->get_signature(),
										LANG_JAVA);
							if(call_id_to_identifier.count(cid) > 0) {

								CGNode* callee_node = get_node(
										call_id_to_identifier[cid]);
								cur_node->add_callee(callee_node, true);
								if(DEBUG)
									cout << "addding call to super: " << endl;
							}
							else
							{
								if(DEBUG)
									cout << "no identifier for call id" << endl;
							}
						}
						else
						{
							if(DEBUG) cout << "Super method not found" << endl;
						}



					}
					else if(fc->is_virtual_call())
					{
						cout << "virtual call*********" << endl;
						cout << "SIGNATURE: " << fc->get_signature()->to_string() << endl;
						set<call_id> targets;
						fc->get_virtual_call_targets(targets);
						set<call_id>::iterator it = targets.begin();
						for(; it!= targets.end(); it++)
						{
							call_id ci = *it;
							cout << "******* looking for ******" << endl;
							cout << "call id: " << ci.to_string() << endl;
							cout << "type: " << ci.sig->to_string() << " add:"
									<< ci.sig << endl;
							cout << "*********************" << endl;
							il::type* tt = ci.sig;
							assert(tt->is_function_type());
							il::function_type* ft = static_cast<il::function_type*>(tt);
							//if(ft->ret_type!= NULL) cout << "ret type: " << *ft->ret_type << " add: " << ft->ret_type << endl;
							//if(ft->arg_types.size()>0) cout << "arg type: " << *ft->arg_types[0] << " add: " << ft->arg_types[0] << endl;

							/*auto it = call_id_to_identifier.begin();
							for(; it !=call_id_to_identifier.end(); it++) {
								call_id cur = it->first;
								cout << "call id in map: " << cur.to_string() << endl;
								cout << "type: " << cur.sig->to_string() << " add:"
										<< cur.sig << endl;

								il::type* tt = cur.sig;
								assert(tt->is_function_type());
								il::function_type* ft = static_cast<il::function_type*>(tt);
								if(ft->ret_type!= NULL) cout << "ret type: " << *ft->ret_type << " add: " << ft->ret_type << endl;
								if(ft->arg_types.size()>0) cout << "arg type: " << *ft->arg_types[0] << " add: " << ft->arg_types[0] << endl;
							}*/
							//cout << "looking for call id: " << ci.to_string() << endl;
							if(call_id_to_identifier.count(ci) > 0) {
								cout << "identifier found!!!" << endl;
								CGNode* callee_node = get_node(
										call_id_to_identifier[ci]);
								cur_node->add_callee(callee_node, false);
								cout << "adding to " << cur_node->id <<
										" callee: " << callee_node->id << endl;

							}

						}
					}
					else {

						cout << "non virtual call " << endl;
						string callee_name = fc->get_function_name();
						il::type* sig = fc->get_signature();
						cout << "SIGNATURE: " << sig->to_string() << endl;


						call_id ci(callee_name, fc->get_namespace(), sig, cur_lang);
						cout << "looking for call id: " << ci.to_string() << endl;

						cout << "name: " << callee_name << endl;
						cout << "ns: " << fc->get_namespace().to_string() << endl;
						cout << "sig: " << sig->to_string() << endl;
						cout << "cur lang: " << cur_lang << endl;

						if(orig_to_spec.count(ci) > 0) {
							ci = call_id(orig_to_spec[ci]);
						}

						if(call_id_to_identifier.count(ci) > 0) {
							cout << "call id found static!" << endl;
							CGNode* callee_node = get_node(call_id_to_identifier[ci]);
							cur_node->add_callee(callee_node, true);

						}
					}
				}

				else if(!process_function_address &&
						inst_type == FUNCTION_POINTER_CALL)
				{
					FunctionPointerCall* fpc = (FunctionPointerCall*) cur;
					il::type* sig = fpc->get_function_pointer()->get_type()->
							get_deref_type();
					if(signature_to_call_id.count(sig) > 0) {
						set<call_id>& targets = signature_to_call_id[sig];
						set<call_id>::iterator it = targets.begin();
						for(; it!= targets.end(); it++)
						{
							call_id ci = *it;
							if(call_id_to_identifier.count(ci) > 0) {
								CGNode* callee_node = get_node(
										call_id_to_identifier[ci]);
								cur_node->add_callee(callee_node, false);

							}

						}

					}


				}

				else if(process_function_address && inst_type == ADDRESS_LABEL)
				{
					AddressLabel* al = (AddressLabel*) cur;
					if(!al->is_function_label()) continue;
					string fn_name = al->get_label();
					il::type* t = al->get_signature();
					call_id ci(fn_name, il::namespace_context(), t, cur_lang);
					signature_to_call_id[t].insert(ci);
				}

			}
		}

		if(b == exit_b) continue;
		set<CfgEdge*>& succs = b->get_successors();
		set<CfgEdge*>::iterator it = succs.begin();
		for(; it!= succs.end(); it++)
		{
			CfgEdge* e = *it;
			sail::Block* t = e->get_target();
			worklist.insert(t);
		}



	}
	if(DEBUG)
	{
		cout << "######## DONE ###### " << endl;
	}

}

map<il::type*, set<call_id> >&
	Callgraph::get_signature_to_callid_map()
{
	return this->signature_to_call_id;
}

void Callgraph::build_initial_callgraph(set<string>& sail_serial_ids)
{
	set<string>::iterator it = sail_serial_ids.begin();
	for(; it!= sail_serial_ids.end(); it++)
	{
		const string& cur_id = *it;
		if(DEBUG) cout << "Processing function " << cur_id << endl;
		sail::Function* f= (sail::Function*) dm->get_data(cur_id, SAIL_FUNCTION);
		cout << "Function signature: " <<
				f->get_signature()->to_string() << endl;
		assert(f!= NULL);
		process_summary_unit(f, true);
		dm->mark_unused(cur_id, SAIL_FUNCTION);
	}

	it = sail_serial_ids.begin();
	for(; it!= sail_serial_ids.end(); it++)
	{
		const string& cur_id = *it;
		sail::Function* f= (sail::Function*) dm->get_data(cur_id, SAIL_FUNCTION);
		assert(f!= NULL);
		process_summary_unit(f, false);
		dm->mark_unused(cur_id, SAIL_FUNCTION);
	}
}

Callgraph::Callgraph(set<string>& serial_ids, DataManager* dm,
		map<string, set<Error*> > & errors):errors(errors)
{
	read_fields.clear();
	written_fields.clear();
	this->dm = dm;
	checks = NULL;
	this->track_dynamic_types = false;
	map_call_ids_to_identifiers(serial_ids);
	build_initial_callgraph(serial_ids);
	assign_ids_postorder();
	compute_dominators();
	remove_replaced_nodes();

}




void Callgraph::remove_replaced_nodes()
{
	set<Identifier>::iterator it = ids_with_spec.begin();
	for(; it!= ids_with_spec.end(); it++)
	{
		Identifier cur = *it;
		if(nodes.count(cur) == 0) continue;
		CGNode* node = get_node(cur);
		set<CGNode*> callees;
		get_exclusive_callees(node, callees);
		set<CGNode*>::iterator it2 = callees.begin();
		for(; it2!= callees.end(); it2++) {
			CGNode* cur_callee = *it2;
			remove_node(cur_callee);
		}
	}

}



bool Callgraph::is_entry_point(CGNode* node)
{
	return entry_points.count(node) > 0;
}

void Callgraph::assign_ids_postorder()
{
	// id is reserved for optional initialization function
	int cur_id = 1;
	set<CGNode*> not_visited;
	find_entry_points(not_visited);
	set<CGNode*> visited;
	set<CGNode*>::iterator it = entry_points.begin();
	for(; it!= entry_points.end(); it++)
	{
		assign_ids_postorder(*it, visited, cur_id);
	}
	for(it =visited.begin(); it!= visited.end(); it++){
		not_visited.erase(*it);
	}

	for(it = not_visited.begin(); it!= not_visited.end(); it++)
	{
		assign_ids_postorder(*it, visited, cur_id);
	}
	mark_backedges();
}

void Callgraph::compute_dominators()
{
	set<CGNode*, CGNodeLessThan> ordered_nodes;
	{
		map<Identifier, CGNode*>::iterator it = this->nodes.begin();
		for(; it!= this->nodes.end(); it++) {
			ordered_nodes.insert(it->second);
		}
	}



	set<CGNode*, CGNodeLessThan>::iterator nodes_it = ordered_nodes.begin();
	for(; nodes_it!= ordered_nodes.end(); nodes_it++){
		CGNode* cur = (*nodes_it);
		if(cur->has_callers()){
			dominators[cur].insert(ordered_nodes.begin(), ordered_nodes.end());
		}
	}


	while(!ordered_nodes.empty())
	{
		CGNode* cur = *ordered_nodes.rbegin();




		set<CGNode*> doms_pred;
		const map<CGNode*, cg_edge_id>& preds = cur->get_callers();
		map<CGNode*, cg_edge_id>::const_iterator it = preds.begin();
		bool first = true;
		for(; it!=preds.end(); it++){
			CGNode* cur_pred = it->first;
			set<CGNode*>& pred_doms = this->dominators[cur_pred];
			if(first){
				first = false;
				set<CGNode*>::iterator pred_doms_it = pred_doms.begin();
				for(; pred_doms_it!= pred_doms.end(); pred_doms_it++){
					CGNode* b = *pred_doms_it;
					doms_pred.insert(b);
				}

				continue;
			}


			set<CGNode*> intersection;
			set_intersection(doms_pred.begin(), doms_pred.end(), pred_doms.begin(),
					pred_doms.end(), insert_iterator<set<CGNode*> >
					(intersection, intersection.begin()) );
			doms_pred.clear();
			doms_pred.insert(intersection.begin(), intersection.end());

		}

		set<CGNode*> doms;
		set_intersection(doms_pred.begin(), doms_pred.end(),
				dominators[cur].begin(),dominators[cur].end(),
				insert_iterator<set<CGNode*> >
							(doms, doms.begin()) );

		doms.insert(cur);


		// If dominators changed, update dominators and enqueue any succs.
		if(dominators[cur].size()!= doms.size() ){
			dominators[cur] = doms;
			const map<CGNode*, cg_edge_id>& succs = cur->get_callees();
			map<CGNode*, cg_edge_id>::const_iterator it2;
			for(it2 = succs.begin(); it2!=succs.end(); it2++){
				ordered_nodes.insert(it2->first);
			}

		}
		ordered_nodes.erase(cur);
	}


}

set<CGNode*>& Callgraph::get_global_initializers()
{
	return this->init_functions;
}

void Callgraph::mark_backedges()
{
	map<Identifier, CGNode*>::iterator it = nodes.begin();
	for(; it!= nodes.end(); it++) {
		CGNode* cur = it->second;
		int cur_id = cur->cg_id;
		map<CGNode*, cg_edge_id>::iterator it2 = cur->callees.begin();
		for(; it2!=cur->callees.end(); it2++){
			CGNode* callee = it2->first;
			int callee_id = callee->cg_id;
			// Backedge found from cur to callee
			if(callee_id >= cur_id){
				assert(cur->callees.count(callee)>0);
				assert(callee->callers.count(cur)>0);
				cur->callees[callee].mark_backedge();
				callee->callers[cur].mark_backedge();

			}
		}
	}
}

void Callgraph::assign_ids_postorder(CGNode* cur, set<CGNode*>& visited,
			int& cur_id)
{
	if(visited.count(cur)>0) return;
	visited.insert(cur);
	map<CGNode*, cg_edge_id>::iterator it = cur->callees.begin();
	for(; it!= cur->callees.end(); it++)
	{
		CGNode* callee = it->first;
		assign_ids_postorder(callee, visited, cur_id);
	}
	cur->cg_id = ++cur_id;
}



void Callgraph::find_entry_points(set<CGNode*>& all_nodes)
{
	//cout << "************* looking for entry points **********" << endl;
	set<Identifier> to_delete;
	map<Identifier, CGNode*>::iterator it = nodes.begin();
	for(; it!= nodes.end(); it++){
		CGNode* cur = it->second;
	//	cout << "CUR node: " << cur->id << endl;
		Identifier id = it->first;
		bool deleted = false;
		if(init_functions.count(cur) > 0) continue;

		//delete me
		/*
		cout << "ID: " << id.to_string() << endl;
		cout << "NS: " << id.get_namespace().to_string() << endl;
		if(id.get_namespace().to_string().find("LtuioDroid/osc/OSCInterface") != string::npos) {
			cout << "ID deleted: " << id.to_string() << endl;
			to_delete.insert(id);
			continue;
		}*/
	//delete me end


	//	cout << "CUR node 0: " << cur->id << endl;

		all_nodes.insert(cur);
		string fun_name = id.get_function_name();



		for(int i=0; functions_to_ignore[i]!=""; i++)
		{
			if(fun_name.find(functions_to_ignore[i])!=string::npos)
			{
					to_delete.insert(id);
					deleted = true;
					break;
			}
		}
		//cout << "CUR node 2: " << cur->id << endl;
		if(deleted) continue;
		//cout << "CUR node 3: " << cur->id << endl;
		if(!cur->has_callers())
		{

			cout << "CUR node 3: " << cur->id << endl;
			if(fun_name == string(SPECIFY_CHECKS))
			{
				checks = cur;
				to_delete.insert(id);
			}
			else if(fun_name.find(POSITION_DEPENDENT_ADT_PREFIX)
					!=string::npos)
			{
				to_delete.insert(id);
			}
			else if(fun_name.find(SINGLE_VALUED_ADT_PREFIX)!=string::npos)
			{

				to_delete.insert(id);
			}
			else if(fun_name.find(MULTI_VALUED_ADT_PREFIX)!=string::npos)
			{

				to_delete.insert(id);
			}
			else
			{
				cout << "CUR node 4: " << cur->id << endl;
				entry_points.insert(cur);


			}
		}

	}

	//all entry points are called by all the init functions, if any
	set<CGNode*>::iterator it2 = init_functions.begin();
	for(; it2!= init_functions.end(); it2++)
	{
		CGNode* i = *it2;
		set<CGNode*>::iterator it3 = entry_points.begin();
		for(; it3 != entry_points.end(); it3++)
		{
			CGNode* e = *it3;
			e->add_callee(i, true);
		}
	}



	set<Identifier>::iterator del_it = to_delete.begin();
	for(; del_it!=to_delete.end(); del_it++){
		nodes.erase(*del_it);
	}
}



CGNode* Callgraph::get_node(const Identifier& id)
{
	if(nodes.count(id) == 0){
		CGNode* node = new CGNode(id);
		nodes[id]=node;
		return node;
	}

	return nodes[id];

}

string escape_fun_id(const string & s)
{
	string res;
	for(unsigned int i=0; i<s.size(); i++) {
		if(s[i]=='<') continue;
		if(s[i]=='>') continue;
		if(s[i]=='-') continue;
		if(s[i]==';') continue;
		if(s[i]=='(') continue;
		if(s[i]==')') continue;
		res += s[i];
	}
	return res;

}



string Callgraph::to_dotty(const string& cur_dir)
{
	string res = "digraph G { \n";
	map<Identifier, CGNode*>::iterator it = nodes.begin();
	for(; it!=nodes.end(); it++){
		string fun_id= it->first.to_string();
		if(fun_id.size() > cur_dir.size() && it->first.get_language()!=LANG_JAVA)
			fun_id = fun_id.substr(cur_dir.size());
		if(it->first.get_language()==LANG_JAVA)
		{
			if(fun_id.rfind('(')!=string::npos){
				if(fun_id.rfind('/')!=string::npos){
					fun_id = fun_id.substr(fun_id.substr(0,
							fun_id.rfind('(')).rfind('/')+1);
				}
			}
		}
		CGNode* node = it->second;
		string cg_id_str = int_to_string(node->cg_id);
		string label = cg_id_str + ":" + fun_id;
		res += "node" + cg_id_str + " [shape = box]";
		if(entry_points.count(node) > 0) res+="[color=red]";
		res+="[label= \"" +
				label + "\"] \n";

		map<CGNode*, cg_edge_id>& callees = node->callees;
		map<CGNode*, cg_edge_id>::iterator it = callees.begin();
		for(; it!=callees.end(); it++)
		{
			CGNode* callee = it->first;
			string callee_id = int_to_string(callee->cg_id);
			cg_edge_id edge_id = it->second;


			res+= "node" +  cg_id_str + " -> " +
					"node" + callee_id;

			if(!edge_id.is_definite()) {
				res += "[color=blue]";
			}
			res+= "\n";
		}
	}
	res+= "}";

	cout << "*************** CG ****************" << endl;

	cout << res << endl;

	return res;

}

set<CGNode*> & Callgraph::get_entry_points()
{
	return entry_points;
}

map<Identifier, CGNode*>& Callgraph::get_nodes()
{
	return nodes;
}

Callgraph::~Callgraph()
{
	map<Identifier, CGNode*>::iterator it = nodes.begin();
	for(; it!= nodes.end(); it++){
		delete it->second;
	}
	//set<CGNode*>::iterator it2 = init_functions.begin();
	//for(; it2!= init_functions.end(); it2++)
		//delete *it2;

}

bool SignatureLessThan::operator( )(const il::type* const & sig1,
	   const il::type* const & sig2) const
{
	return sig1->to_string() < sig2->to_string();
}





