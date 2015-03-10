/*
 * Cfg.cpp
 *
 *  Created on: Jul 6, 2008
 *      Author: isil
 *  This class constructs the CFG, applies our transformation, separates loops
 *  cleanly and computes dominators. It also numbers all nodes in reverse
 *  post-order for later analysis.
 */

#include "Cfg.h"
#include "BasicBlock.h"
#include "Instruction.h"
#include "CfgEdge.h"
#include "T1T2Block.h"
#include "Jump.h"
#include "Branch.h"
#include "Symbol.h"
#include "FunctionCall.h"
#include "Loop.h"
#include "SuperBlock.h"
#include "Function.h"
#include <assert.h>
#include <queue>
#include "util.h"
#include <algorithm>
#include "ReachingDefInfer.h"
#include "PreciseTypeInfer.h"
#include "TypeInference.h"



#include <iostream>

/*
 * Enables constant consistency checking of the CFG.
 */
#define CONSISTENCY_CHECKING true
#define USING_TYPE_INFERENCE true

using namespace std;


namespace sail {

Cfg::Cfg()
{
	this->f = NULL;
}

Cfg::Cfg(Function* f, bool exit_fn_modifies_control) {
	this->f = f;
	this->exit_fn_as_exception = exit_fn_modifies_control;
	build_cfg(f->get_body());

	assert(unresolved_blocks.size()==0);

	// Give preliminary id's to identify unreachable blocks
	assign_block_ids();

	// Remove unreachable, empty, and redundant blocks.
	optimize_cfg();

	// Use node-splitting to make the CFG reducible.
	make_reducible();

	// Need to re-id because node splitting gives copied nodes the
	// same id as the original node.
	assign_block_ids();
	//haiyan open the print
	//cout << blocks_to_dotty() << endl;
	//haiyan end




	if(CONSISTENCY_CHECKING)
		check_cfg();
	get_edges(edges);
	identify_loops();
	if(CONSISTENCY_CHECKING)
		check_cfg();
	assign_block_ids();



	//I would like to check if it is my problem of the parameter type is missing
	if (0) {
		cout
				<< "TESTING the body of function :::  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> "
				<< endl;
		vector<Instruction*>* bdy = f->get_body();
		vector<Instruction*>::iterator it = bdy->begin();
		for (; it != bdy->end(); it++) {
			cout << "INS ::  >>>> " << (*it)->to_string() << endl;
		}
	}


	//haiyan check the type of Leu/domob/angulo/Vector;
//		cerr << "testing here " << endl;
//		il::type* t = il::type::get_type_from_name("Landroid/content/pm/ApplicationInfo;");
//		cerr << "type is " << t->to_string() << endl;
//		if(t->is_record_type()){
//			cout << "ele size " << ((record_type*)t)->elems.size() << endl;
//			if(1){
//				vector<record_info*> elems = ((record_type*)t)->elems;
//				for(vector<record_info*>::iterator it = elems.begin();
//						it != elems.end(); it++){
//					cerr << "Field offset " << (*it)->offset << endl;
//					cerr << "Field name " << (*it)->fname << endl;
//					cerr << "Field type " << (*it)->t->to_string() << endl;
//
// 				}
//				//exit(1);
//
//			}else{
//
//			cerr << "????????// here  " << endl;
//			map<int, il::record_type*> bases =((il::record_type*)t)->get_bases();
//			cerr << "size of its base is " << bases.size() << endl;
//			map<int, il::record_type*>::const_iterator it = bases.begin();
//			for(; it != bases.end(); it ++){
//				cerr << "key is " << it->first << endl;
//				cerr << "base is " << it->second->to_string() << endl;
//			}
//			}
//		}


	//type inference
	if(USING_TYPE_INFERENCE)
		type_inference();


    cout << f->to_string() << endl;
   // cout << "End of type_inference !" << endl;



	// Replace loops with superblocks.
	finalize_cfg();



	if(CONSISTENCY_CHECKING)
		check_cfg();


	reset_loop_count();

}

void Cfg::finalize_cfg()
{
	make_superblocks();
	set<Block*> worklist;
	worklist.insert(this->entry_block);
	while(!worklist.empty())
	{
		Block *b = *(worklist.begin());
		worklist.erase(b);
		blocks.insert(b);
		set<CfgEdge*>::iterator it = b->get_successors().begin();
		for(; it!= b->get_successors().end(); it++)
		{
			worklist.insert((*it)->get_target());
		}
	}
	blocks.insert(exception_block);
	blocks.insert(exit_block);
}

void Cfg::make_superblocks()
{

	if(CONSISTENCY_CHECKING)
		assert(check_cycle(true));

	set<Loop*, LoopCompare>::iterator lit = loops.begin();
	for(; lit!=loops.end(); lit++){
		Loop* cur = *lit;
		if(cur->get_header() == this->entry_block)
		{
			BasicBlock* new_entry = new BasicBlock();
			basic_blocks.insert(new_entry);
			CfgEdge* e = new CfgEdge(new_entry, entry_block, NULL);
			new_entry->add_successor_edge(e);
			entry_block->add_predecessor_edge(e);
			entry_block = new_entry;
			assign_block_ids();
		}
		if(CONSISTENCY_CHECKING)
			assert(check_cycle(true));
		SuperBlock* sb = new SuperBlock(cur, f);
		id_to_superblock[sb->get_identifier().to_string()] = sb;

		super_blocks_ordered.push_back(sb);

		if(CONSISTENCY_CHECKING)
			assert(check_cycle(true));
		this->headers_to_superblocks[cur->get_header()] =sb;


	}
}

const vector<SuperBlock*>& Cfg::get_ordered_superblocks()
{
	return super_blocks_ordered;
}

void Cfg::identify_loops()
{
	compute_dominators();
	compute_post_dominators();
	find_backedges();

	if(CONSISTENCY_CHECKING)
		assert(check_cycle(false));

	set<CfgEdge*>::iterator it = edges.begin();
	for(; it!=edges.end(); it++){
		CfgEdge* cur = *it;
		if(!cur->is_backedge()) continue;
		BasicBlock* header = (BasicBlock*)cur->get_target();
		if(loopheaders.count(header) == 0){
			Loop* l = new Loop(cur);
			loops.insert(l);
			loopheaders[header] = l;
			continue;
		}

		Loop* l = loopheaders[header];
		l->add_backedge(cur);

	}
	set<Loop*, LoopCompare>::iterator li1 = loops.begin();
	for(; li1!=loops.end(); li1++)
	{
		Loop *cur = *li1;
		set<Loop*, LoopCompare>::iterator li2 = li1;
		for(; li2!=loops.end(); li2++)
		{
			Loop* pot_parent = *li2;
			if(pot_parent->get_body().count(cur->get_header())>0)
				cur->add_parent_loop(pot_parent);
		}
	}

	edges.clear();

	set<Loop*, LoopCompare>::iterator li = loops.begin();
	if(CONSISTENCY_CHECKING)
		assert(check_cycle(false));


	for(; li!=loops.end(); li++)
	{
		Loop* l = *li;
		l->make_unique_exit(basic_blocks);
		if(CONSISTENCY_CHECKING)
			check_cfg();
		l->insert_recursive_calls(basic_blocks);
		if(CONSISTENCY_CHECKING)
			check_cfg();
		embedded_basic_blocks.insert(l->get_body().begin(), l->get_body().end());

	}
	if(CONSISTENCY_CHECKING){



		assert(check_cycle(true));
	}
}

class ToposortBlock:public binary_function<BasicBlock*, BasicBlock*, bool> {

public:
	bool operator()(const BasicBlock* b1, const BasicBlock* b2) const
	{
		if(b1->get_block_id()== -1 && b2->get_block_id()==-1){
			if(b2-b1 < 0) return true;
			return false;
		}

		return (b2->get_block_id() < b1->get_block_id());
	}
};



void Cfg::find_backedges()
{
	set<CfgEdge*>::iterator it = edges.begin();
	for(; it!=edges.end(); it++)
	{
		CfgEdge* cur_edge = *it;
		BasicBlock* source = (BasicBlock*)cur_edge->get_source();
		BasicBlock* target = (BasicBlock*)cur_edge->get_target();

		if(source->get_dominators().count(target) > 0){
			cur_edge->mark_backedge();
		}
	}
}

void Cfg::get_edges(set<CfgEdge*>& edges)
{
	set<BasicBlock*>::iterator it = basic_blocks.begin();
	for(; it!=basic_blocks.end(); it++)
	{
		BasicBlock* cur = *it;
		edges.insert(cur->get_successors().begin(), cur->get_successors().end());
		edges.insert(cur->get_predecessors().begin(), cur->get_predecessors().end());
	}

}

static map<BasicBlock*, vector<BasicBlock*> > sm;
static map<BasicBlock*, vector<BasicBlock*> > pm;
static vector<BasicBlock*> b;

static string dot;





bool Cfg::check_cycle(bool edge)
{

	set<Block*> worklist;
	worklist.insert(entry_block);
	int c = 0;
	while(worklist.size() > 0)
	{
		c++;
		if(c > 1000000) return false;
		Block* cur = *worklist.begin();
		worklist.erase(cur);
		set<CfgEdge*>::iterator it = cur->get_successors().begin();
		for(;it != cur->get_successors().end(); it++)
		{
			if(!edge)
			{
				if((*it)->is_backedge())
					continue;
			}
			worklist.insert((*it)->get_target());
		}

	}

	return true;

}


/**
 * Make sure the CFG is consistent
 */
void Cfg::check_cfg()
{
	set<BasicBlock*>::iterator it = basic_blocks.begin();

	for(; it!=basic_blocks.end(); it++)
	{
		Block* cur = *it;
		set<CfgEdge*>::iterator s_it =cur->get_successors().begin();
		for(; s_it!=cur->get_successors().end(); s_it++)
		{
			Block *s = (*s_it)->get_target();
			assert(s->get_predecessors().count(*s_it)>0);
		}

		set<CfgEdge*>::iterator p_it =cur->get_predecessors().begin();
		for(; p_it!=cur->get_predecessors().end(); p_it++)
		{
			Block *p = (*p_it)->get_source();
			assert(p->get_successors().count(*p_it)>0);
		}
	}
}


bool CompareBlock::operator()(const Block* b1, const Block* b2) const
{
	if(b1->get_block_id()== -1 && b2->get_block_id()==-1){
		if(b1-b2 < 0) return true;
		return false;
	}

	return (b1->get_block_id() < b2->get_block_id());
}




void Cfg::compute_post_dominators()
{
	set<BasicBlock*, CompareBlock> ordered_blocks;
	set<BasicBlock*>::iterator blocks_it = basic_blocks.begin();
	set<BasicBlock*> init(basic_blocks);
	for(; blocks_it!=basic_blocks.end(); blocks_it++){
		BasicBlock* cur = (*blocks_it);
		if(cur!= this->exit_block){
			cur->set_post_dominators(init);
		}
		ordered_blocks.insert(cur);
	}

	while(!ordered_blocks.empty())
	{
		BasicBlock* cur = *ordered_blocks.begin();

		set<BasicBlock*> pdoms_succ;
		set<CfgEdge*>& succs = cur->get_successors();
		set<CfgEdge*>::iterator it = succs.begin();
		bool first = true;
		for(; it!=succs.end(); it++){
			BasicBlock* cur = (BasicBlock*)(*it)->get_target();
			set<BasicBlock*>& succ_pdoms = cur->get_post_dominators();
			if(first){
				first = false;
				set<BasicBlock*>::iterator it = succ_pdoms.begin();
				for(; it!= succ_pdoms.end(); it++){
					BasicBlock* b = *it;
					pdoms_succ.insert(b);
				}

				continue;
			}


			set<BasicBlock*> intersection;
			set_intersection(pdoms_succ.begin(), pdoms_succ.end(),
					succ_pdoms.begin(), succ_pdoms.end(),
					insert_iterator<set<BasicBlock*> >
			(intersection, intersection.begin()) );
			pdoms_succ.clear();
			pdoms_succ.insert(intersection.begin(), intersection.end());

		}

		set<BasicBlock*> pdoms;
		set_intersection(pdoms_succ.begin(), pdoms_succ.end(),
				cur->get_post_dominators().begin(),
				cur->get_post_dominators().end(),
				insert_iterator<set<BasicBlock*> >
		(pdoms, pdoms.begin()) );

		pdoms.insert(cur);



		// If post-dominators changed, update post-dominators and
		// enqueue any predecessors.
		if(cur->get_post_dominators().size() != pdoms.size() ){
			cur->set_post_dominators(pdoms);
			set<CfgEdge*>& preds = cur->get_predecessors();
			set<CfgEdge*>::iterator it2;
			for(it2 = preds.begin(); it2!=preds.end(); it2++){
				ordered_blocks.insert((BasicBlock*)(*it2)->get_source());
			}

		}
		ordered_blocks.erase(cur);
	}


}

void Cfg::compute_dominators()
{
	set<BasicBlock*, CompareBlock> ordered_blocks;
	set<BasicBlock*>::iterator blocks_it = basic_blocks.begin();
	set<BasicBlock*> init(basic_blocks);
	for(; blocks_it!=basic_blocks.end(); blocks_it++){
		BasicBlock* cur = (*blocks_it);
		if(cur!= this->entry_block){
			cur->set_dominators(init);
		}
		ordered_blocks.insert(cur);
	}


	while(!ordered_blocks.empty())
	{
		BasicBlock* cur = *ordered_blocks.begin();

		set<BasicBlock*> doms_pred;
		set<CfgEdge*>& preds = cur->get_predecessors();
		set<CfgEdge*>::iterator it = preds.begin();
		bool first = true;
		for(; it!=preds.end(); it++){
			BasicBlock* cur_pred = (BasicBlock*)(*it)->get_source();
			set<BasicBlock*>& pred_doms = cur_pred->get_dominators();
			if(first){
				first = false;
				set<BasicBlock*>::iterator pred_doms_it = pred_doms.begin();
				for(; pred_doms_it!= pred_doms.end(); pred_doms_it++){
					BasicBlock* b = *pred_doms_it;
					doms_pred.insert(b);
				}

				continue;
			}


			set<BasicBlock*> intersection;
			set_intersection(doms_pred.begin(), doms_pred.end(), pred_doms.begin(),
					pred_doms.end(), insert_iterator<set<BasicBlock*> >
			(intersection, intersection.begin()) );
			doms_pred.clear();
			doms_pred.insert(intersection.begin(), intersection.end());

		}

		set<BasicBlock*> doms;
		set_intersection(doms_pred.begin(), doms_pred.end(), cur->get_dominators().begin(),
				cur->get_dominators().end(), insert_iterator<set<BasicBlock*> >
		(doms, doms.begin()) );

		doms.insert(cur);


		// If dominators changed, update dominators and enqueue any succs.
		if(cur->get_dominators().size() != doms.size() ){
			cur->set_dominators(doms);
			set<CfgEdge*>& succs = cur->get_successors();
			set<CfgEdge*>::iterator it2;
			for(it2 = succs.begin(); it2!=succs.end(); it2++){
				ordered_blocks.insert((BasicBlock*)(*it2)->get_target());
			}

		}
		ordered_blocks.erase(cur);
	}
}

void Cfg::make_reducible()
{
	set<T1T2Block*, Compare> heap;
	set<BasicBlock*>::iterator it = basic_blocks.begin();
	for(;it != basic_blocks.end(); it++)
	{

		T1T2Block * t = make_T1T2Block(*it);
		if(t->pred_count == 0)
			continue;
		heap.insert(t);

	}


	while(!heap.empty())
	{
		T1T2Block* cur_t = *(heap.begin());
		if(cur_t->representative_block == NULL)
			cout << "block is empty " <<endl;
		else
			//cout <<"block id: "<<cur_t->representative_block->get_block_id()<< endl;
		if(cur_t->pred_count == 0){
			heap.erase(cur_t);
			continue;
		}
		if(cur_t->pred_count != 1) {
			cerr << "WARNING: Encountered irreducible control flow " << endl;
			cerr << "FIXME: Function treated as empty!" << endl;
			basic_blocks.clear();
			blocks.clear();
			super_blocks_ordered.clear();
			id_to_superblock.clear();
			vector<Instruction*> empty;
			build_cfg(&empty);
			assert(unresolved_blocks.size()==0);
			assign_block_ids();
			optimize_cfg();
			return;
			/*
		    cout << "split nodes "<<endl;
			// Need to split this node

			cur_t->split_node(heap, basic_blocks, this->exit_block,
					this->exception_block);
			if(CONSISTENCY_CHECKING)
				check_cfg();

			continue;
			 */
		}

		T1T2Block* pred = *(cur_t->preds.begin());
		heap.erase(cur_t);
		pred->merge(cur_t, heap);

	}

	clear_map_t1t2();

}

void Cfg::optimize_cfg()
{

	remove_unreachable_blocks();
	remove_empty_blocks();
	merge_redundant_blocks();

}

void Cfg::remove_empty_blocks()
{
	//cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
	//cout << "inside remove_empty_blocks" << endl;
	set<BasicBlock*> to_delete;
	set<BasicBlock*>::iterator it = basic_blocks.begin();
	//cout << "original blocks!!!" << endl;
	for(; it!=basic_blocks.end(); it++)
	{
		BasicBlock* cur = *it;
		//	cout << "basic block .." << cur->to_string() << endl;
		//	cout << "basic block pp:: " << cur->to_string(true)<<endl;
		if(!cur->is_empty_block() || cur == this->entry_block)
			continue;
		set<CfgEdge*>& succs = cur->get_successors();
		assert(succs.size() <= 1);
		if(succs.size() == 0) continue;
		to_delete.insert(cur);

		CfgEdge* e = *succs.begin();
		BasicBlock* succ = (BasicBlock*)e->get_target();
		set<CfgEdge*>& preds = cur->get_predecessors();
		set<CfgEdge*>::iterator it2 = preds.begin();
		set<CfgEdge*> to_add;

		for(; it2 != preds.end(); it2++)
		{
			CfgEdge* pred_succ_edge = *it2;
			pred_succ_edge->set_target(succ);
			to_add.insert(pred_succ_edge);
		}

		set<CfgEdge*>& succ_preds = succ->get_predecessors();
		succ_preds.erase(e);
		succ_preds.insert(to_add.begin(), to_add.end());
		delete e;
	}

	set<BasicBlock*>::iterator it3 = to_delete.begin();
	//cout << "delted empty blocks size is : " << to_delete.size()<<endl;
	//cout << "deleted empty blocks " << endl;
	for(; it3!=to_delete.end(); it3++){
		BasicBlock* d = *it3;
		//cout << "basic block deleted.." << d->to_string() << endl;
		//cout << "basic block deleted pp:: " << d->to_string(true)<<endl;
		basic_blocks.erase(d);
		delete d;
	}
}

void Cfg::merge_redundant_blocks()
{
	set<BasicBlock*> to_delete;
	set<BasicBlock*>::iterator it = basic_blocks.begin();
	//cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
	//cout << "inside merge_redundant_blocks" << endl;
	for(; it!=basic_blocks.end(); it++)
	{
		BasicBlock* cur = *it;
		//cout << "basic block .." << cur->to_string() << endl;
		//cout << "basic block pp:: " << cur->to_string(true)<<endl;
		set<CfgEdge*>& succs = cur->get_successors();

		if(succs.size() != 1) continue;

		CfgEdge* e = *succs.begin();
		BasicBlock* succ = (BasicBlock*)e->get_target();
		if(succ == this->exit_block || succ == this->entry_block ||
				succ == this->exception_block) continue;
		set<CfgEdge*>& preds = succ->get_predecessors();
		assert(preds.size() !=0);
		// can't merge
		if(preds.size() > 1) continue;

		merge_blocks(cur, succ, e);
		//cout << "basic block merged redudancy.." << succ->to_string() << endl;
		//cout << "basic block merged redudancy pp:: " << succ->to_string(true)<<endl;
		to_delete.insert(succ);
	}




	set<BasicBlock*>::iterator it2 = to_delete.begin();
	for(; it2!=to_delete.end(); it2++){
		BasicBlock* temp = *it2;
		basic_blocks.erase(temp);
		delete temp;
	}
}

/**
 * Merge blocks b1 and b2 by appending to b2 to b1 and updating all
 * necessary edges. e is the connecting edge between b1 and b2.
 */
void Cfg::merge_blocks(BasicBlock* b1, BasicBlock* b2, CfgEdge* e)
{

	assert(b2->get_predecessors().count(e)>0);
	assert(b1->get_successors().count(e)>0);
	// First, append all b1 statements to b2
	vector<Instruction*>& stmts = b2->get_statements();
	b1->add_statements(stmts);


	// Update outgoing edges of b1
	b1->get_successors().clear();
	set<CfgEdge*>& succs = b2->get_successors();
	set<CfgEdge*>::iterator it = succs.begin();
	for(; it!= succs.end(); it++)
	{
		CfgEdge* cur_edge = *it;
		cur_edge->set_source(b1);
		b1->get_successors().insert(cur_edge);
	}
	b2->get_successors().clear();

	delete e;

}

void Cfg::remove_unreachable_blocks()
{
	set<BasicBlock*>::iterator it = basic_blocks.begin();
	set<BasicBlock*> to_delete;
	//cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
	//cout << "inside remove_unreachable_blocks" << endl;
	for(; it!= basic_blocks.end(); it++)
	{
		BasicBlock* cur = *it;
		//	cout << "basic block .." << cur->to_string() << endl;
		//	cout << "basic block pp:: " << cur->to_string(true)<<endl;
		if(cur->get_block_id() != INVALID_BLOCK_ID) continue;

		// Never remove the end and exception blocks!
		if(cur == this->exit_block || cur == this->exception_block) continue;


		set<CfgEdge*> succ_edges =cur->get_successors();
		set<CfgEdge*>::iterator it2 = succ_edges.begin();
		for(; it2!= succ_edges.end(); it2++)
		{
			CfgEdge* cur_edge = *it2;
			//		cout << "cfgEdge removed .." << cur_edge->to_string() << endl;
			//		cout << "cfgEdge removed pp:: " << cur_edge->to_string(true)<<endl;
			BasicBlock* succ = (BasicBlock*)cur_edge->get_target();
			set<CfgEdge*>& pred_edges = succ->get_predecessors();
			pred_edges.erase(cur_edge);
			delete cur_edge;
		}

		to_delete.insert(cur);

	}

	set<BasicBlock*>::iterator it3 = to_delete.begin();
	for(; it3!=to_delete.end(); it3++){
		BasicBlock* d = *it3;
		basic_blocks.erase(d);
		d->set_statements_unreachable();
		delete d;
	}
}

void Cfg::print_cfg()
{
	set<BasicBlock*>::iterator it = basic_blocks.begin();
	for(; it!= basic_blocks.end(); it++)
	{
		BasicBlock* cur = *it;
		cout << "******BEGIN BLOCK*********" << endl;
		cout << cur->to_string() << endl;
		cout << "******END BLOCK*********" << endl;
	}

	cout << "END BLOCK ID: " << this->exit_block->get_block_id() << endl;
}

set<Block*>& Cfg::get_blocks()
{
	return this->blocks;
}
set<BasicBlock*>& Cfg::get_basic_blocks()
{
	return this->basic_blocks;
}


string Cfg::to_dotty(bool pretty_print)
{

	string res = "digraph G { \n";
	set<Block*>::iterator it = blocks.begin();
	for(; it!= blocks.end(); it++){
		Block* b = *it;
		assert(b!=NULL);
		string block_string = "";
		if(b == this->entry_block) block_string += "ENTRY BLOCK \\n";
		if(b == this->exit_block) block_string += "EXIT BLOCK \\n";
		if(b == this->exception_block) block_string += "EXCEPTION BLOCK \\n";
		res+= b->to_dotty(block_string, pretty_print);
	}
	res+= "}";
	return res;
}

BasicBlock* Cfg::get_entry_block()
{
	return entry_block;
}
BasicBlock* Cfg::get_exit_block()
{
	return exit_block;
}
BasicBlock* Cfg::get_exception_block()
{
	return exception_block;
}

SuperBlock* Cfg::get_superblock(const Identifier & id)
{
	if(id_to_superblock.count(id.to_string()) == 0) return NULL;
	return id_to_superblock[id.to_string()];
}

string Cfg::blocks_to_dotty()
{
	cout << "BLOCKS MAP SIZE: " << this->basic_blocks.size() << endl;
	string res = "digraph G { \n";
	set<BasicBlock*>::iterator it = this->basic_blocks.begin();
	for(; it!= basic_blocks.end(); it++){
		BasicBlock* b = (*it);
		long block_id = b->get_block_id();
		if(b->get_block_id() == -1){
			block_id =  (long)b;
			int a = block_id;
			block_id = a<0? -a:a;

		}


		string block_string = "";

		block_string += "Dominator: ";
		set<BasicBlock*>::iterator i1 = b->get_dominators().begin();
		for(; i1 != b->get_dominators().end(); i1++)
		{
			block_string += int_to_string((*i1)->get_block_id()) + " ";
		}
		block_string += "\\n";
		/*
		block_string += "\\nReachable: ";
		set<BasicBlock*>::iterator i2 = b->get_reachable_blocks().begin();
		for(; i2 != b->get_reachable_blocks().end(); i2++)
		{
			block_string += int_to_string((*i2)->get_block_id()) + " ";
		}
		block_string += "\\n";
		 */

		if(b == this->entry_block) block_string += "ENTRY BLOCK \\n";
		if(b == this->exit_block) block_string += "EXIT BLOCK \\n";
		if(b == this->exception_block) block_string += "EXCEPTION BLOCK \\n";
		block_string += escape_string(line_wrap(b->to_string(false), 50));
		/*
		cout << "SSSSSS *************************** " << endl;

		string res = b->to_string(false);
		cout << res << endl;
		for(int i=0; i < res.size(); i++)
		{
			cout << (unsigned int)res[i] << "  ";
		}
		cout << endl;
		 */


		set<CfgEdge*>& succs = b->get_successors();
		res += "node" + int_to_string(block_id) + " [shape = box][label= \"" +
				block_string + "\"] \n";

		set<CfgEdge*>::iterator succs_it = succs.begin();

		for(; succs_it!=succs.end(); succs_it++){
			CfgEdge* succ_edge = *succs_it;
			string edge_style = succ_edge->is_backedge() ? "[style=dotted]" : "";
			assert(basic_blocks.count((BasicBlock*)succ_edge->get_target())>0);
			long succ_id = succ_edge->get_target()->get_block_id();
			if(succ_id == -1) succ_id = (long) succ_edge->get_target();
			res+= "node" + int_to_string(block_id) + " -> " +
					"node" + int_to_string(succ_id) + " [label = \"" +
					(succ_edge->get_cond() ? escape_string(succ_edge->get_cond()->to_string(false)) : "true") +
					"\"]" + edge_style + "\n";
		}

	}
	res+= "}";
	return res;
}



/**
 * Assign block id's in reverse post order.
 */

void Cfg::assign_block_ids()
{
	int cur_id = basic_blocks.size();
	set<BasicBlock*> visited;
	assert(this->entry_block != NULL);
	traverse_postorder(this->entry_block, cur_id, visited);

}

void Cfg::traverse_postorder(BasicBlock* cur, int& counter,
		set<BasicBlock*>& visited)
{
	if(visited.count(cur) > 0) return;
	visited.insert(cur);

	set<CfgEdge*>& _succs = cur->get_successors();
	multiset<CfgEdge*, CompareEdge> succs;


	succs.insert(_succs.begin(), _succs.end());
	multiset<CfgEdge*, CompareEdge>::iterator it = succs.begin();
	//haiyan added for test
	unsigned int i = 0;
	for(; it!=succs.end(); it++)
	{
		BasicBlock* cur_succ = (BasicBlock*)(*it)->get_target();
		if(visited.count(cur_succ) > 0) continue;
		traverse_postorder(cur_succ, counter, visited);
		//haiyan added for test
		i++;
	}

	cur->set_block_id(counter--);
}



void Cfg::build_cfg(vector<Instruction*>* function_body)
{

	int cur_index = 0;
	this->entry_block= NULL;
	this->exception_block = get_new_block(NULL);
	if(function_body->size()>=1) {
		Instruction* first = (*function_body)[0];

		//skip the first label
		if(first->get_instruction_id() == SAIL_LABEL){
			cur_index++;
			this->entry_block = get_new_block(first);
		}
	}
	if(entry_block == NULL)
		this->entry_block = new BasicBlock();
	basic_blocks.insert(entry_block);
	//cur_index is the index that find sail_label, JUMP, BRANCH...)
	cur_index = fill_basic_block(entry_block, function_body, cur_index);

	BasicBlock* cur_block = entry_block;
	cout << "index is " << cur_index << endl;
	//must satisfying the condition(index of label is less than the size of body)
	while(cur_index < (int) function_body->size())
	{
		Instruction* cur_inst = (*function_body)[cur_index];
		int id = cur_inst->get_instruction_id();

		switch(id)
		{
		case SAIL_LABEL:
		{
			BasicBlock* new_block = get_new_block(cur_inst);
			connect_blocks(cur_block, new_block, NULL);
			cur_index = fill_basic_block(new_block, function_body, cur_index+1);
			cur_block = new_block;
			break;
		}
		case JUMP:
		{
			Jump* j = (Jump*) cur_inst;
			Label* l = j->get_label();
			process_jump_label(cur_block, l, NULL);
			process_next_block(cur_block, cur_index, function_body);
			break;
		}
		case BRANCH:
		{
			Branch* br = (Branch*) cur_inst;
			vector<pair<Symbol*, Label*> > * targets = br->get_targets();
			for(int i=0; i<(int) targets->size(); i++) {
				pair<Symbol*, Label*> cur_target = (*targets)[i];
				process_jump_label(cur_block, cur_target.second,
						cur_target.first);
			}
			process_next_block(cur_block, cur_index, function_body);
			break;
		}

		/**
		 * This deals with exit functions.
		 */
		case FUNCTION_CALL:
		{
			FunctionCall* fc = (FunctionCall*) cur_inst;
			// We should never have gotten here if this was a non-exit fn.
			assert(fc->is_exit_function());
			connect_blocks(cur_block, exception_block, NULL);

			process_next_block(cur_block, cur_index, function_body);

			break;
		}
		case EXCEPTION_RETURN:
		{
			connect_blocks(cur_block, exception_block, NULL);
			process_next_block(cur_block, cur_index, function_body);
			break;
		}
		default:
		{
			assert(false);
		}
		}

	}
	if(cur_block->get_successors().size() > 0)
	{
		/*
		 * Deal with the case where the exit function or jump is
		 * the last statement in the function.
		 */
		cur_block = get_new_block(NULL);
	}
	this->exit_block = cur_block;

}

void Cfg::process_jump_label(BasicBlock* cur_block, Label* l, Symbol* cond)
{
	BasicBlock* succ = resolved_blocks[l];
	CfgEdge* e = connect_blocks(cur_block, succ, cond);
	if(succ == NULL){
		add_unresolved_edge(l, e);
	}
}

void Cfg::process_next_block(BasicBlock*& cur_block, int& cur_index,
		vector<Instruction*>* function_body)
{

	if(++cur_index >= (int) function_body->size()) return;
	Instruction* next_inst = (*function_body)[cur_index];
	if(next_inst->get_instruction_id() == SAIL_LABEL){
		cur_index++;
	}
	cur_block = get_new_block(next_inst);
	cur_index = fill_basic_block(cur_block, function_body, cur_index);
}

void Cfg::add_unresolved_edge(Label* l, CfgEdge* e)
{
	if(unresolved_blocks.count(l) == 0)
	{
		set<CfgEdge*> s;
		s.insert(e);
		unresolved_blocks[l] = s;
		return;
	}

	set<CfgEdge*> &edges = unresolved_blocks[l];
	edges.insert(e);
}

CfgEdge* Cfg::connect_blocks(BasicBlock* pred, BasicBlock* succ, Symbol* cond)
{
	CfgEdge* edge = new CfgEdge(pred, succ, cond);
	pred->add_successor_edge(edge);
	if(succ!= NULL) succ->add_predecessor_edge(edge);
	return edge;
}

BasicBlock* Cfg::get_new_block(Instruction* inst)
{
	if(inst == NULL || inst->get_instruction_id()!=SAIL_LABEL){
		BasicBlock* b = new BasicBlock();
		basic_blocks.insert(b);
		return (b);
	}

	Label* l = (Label*) inst;
	BasicBlock* b = new BasicBlock();
	basic_blocks.insert(b);
	resolved_blocks[l] = b;

	// resolve any dependencies
	if(unresolved_blocks.count(l) == 0) return b;
	set<CfgEdge*>& dependencies  = unresolved_blocks[l];
	set<CfgEdge*>::iterator it = dependencies.begin();
	for(; it!=dependencies.end(); it++)
	{
		CfgEdge* cur = *it;
		cur->set_target(b);
		b->add_predecessor_edge(cur);
	}
	unresolved_blocks.erase(l);
	return b;
}

int Cfg::fill_basic_block(BasicBlock* cur_block, vector<Instruction*>* body,
		int start_index)
{
	int i;
	for(i=start_index; i<(int)body->size(); i++)
	{
		Instruction* cur_inst = (*body)[i];
		if(is_control_instruction(cur_inst))
			return i;
		cur_block->add_statement(cur_inst);
	}

	return i;
}

bool Cfg::is_control_instruction(Instruction* inst)
{
	int id = inst->get_instruction_id();
	if(id == SAIL_LABEL || id == JUMP || id == BRANCH || id == EXCEPTION_RETURN)
		return true;

	if(!this->exit_fn_as_exception)
		return false;

	if(id != FUNCTION_CALL) return false;
	FunctionCall* fc = (FunctionCall*) inst;
	return (fc->is_exit_function());
}

bool Cfg::is_header_of_superblock(BasicBlock* b)
{
	return headers_to_superblocks.count(b) > 0;
}
SuperBlock* Cfg::get_superblock_of_entry_block(BasicBlock* b)
{
	assert(headers_to_superblocks.count(b) > 0);
	return headers_to_superblocks[b];
}


void Cfg::type_inference()
{
	//cout << "===============  type inference start ================ " << endl;



	//cout << "function  : "  << f->to_string();

	Identifier id = f->get_identifier();
	language_type lang = id.get_language();
	if(lang == LANG_JAVA)
	{
//		TypeInference* ti = new TypeInference(this->entry_block,
//				this->exit_block, this->exception_block);
		//changed by haiyan
		TypeInference* ti = new TypeInference(f,
				this->entry_block,
				this->exit_block,
				this->exception_block);

		ti->assign_new_type();
		delete ti;
	}

	//cout << "===============  type inference end ================= " << endl;
}




Cfg::~Cfg() {

}

bool CompareEdge::operator()(const CfgEdge* e1, const CfgEdge* e2) const
{
	Block* b1 = ((CfgEdge*)e1)->get_target();
	Block* b2 = ((CfgEdge*)e2)->get_target();
	BasicBlock* bb1;
	if(!b1->is_superblock())
		bb1 = (BasicBlock*) b1;
	else bb1 = ((SuperBlock*)b1)->get_entry_block();
	BasicBlock* bb2;
	if(!b2->is_superblock())
		bb2 = (BasicBlock*) b2;
	else bb2 = ((SuperBlock*)b2)->get_entry_block();

	string s1 = bb1->to_string(false, false);
	string s2 = bb2->to_string(false, false);
	return s1 < s2;
}

}
