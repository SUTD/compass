#include "AllocContextController.h"
#include "MemNode.h"
/******
 * AllocContext
 */
AllocContext::AllocContext() {
	alloc_node = NULL;
	alloc_ins = NULL;
	this->lhs_ap = NULL;
//	constructor = NULL;
}

void AllocContext::set_alloc_ins(sail::FunctionCall* alloc) {
	assert(alloc != NULL);
	alloc_ins = alloc;
}

void AllocContext::set_alloc_lhs_ap(AccessPath* lhs_ap){
	assert(lhs_ap != NULL);
	this->lhs_ap = lhs_ap;
}

//void AllocContext::set_constructor(sail::FunctionCall* constr) {
//	assert(constr != NULL);
//	constructor = constr;
//}



void AllocContext::set_alloc_memnode(MemNode* mnode) {
	assert(mnode != NULL);
	alloc_node = mnode;
}


sail::FunctionCall* AllocContext::get_alloc_ins() const {
	return alloc_ins;
}

AccessPath* AllocContext::get_alloc_lhs_ap() const{
	assert(this->lhs_ap != NULL);
	return this->lhs_ap;
}

//sail::FunctionCall* AllocContext::get_constructor() const {
//	return constructor;
//}



MemNode* AllocContext::get_alloc_mem_node() const {
	return alloc_node;
}



const string AllocContext::to_string() const {
	string str;
	str += "Alloc :: ";
	if(alloc_ins != NULL)
		str += alloc_ins->to_string();

//	if (constructor != NULL) {
//		str += "Constructor :: ";
//		str += constructor->to_string();
//	}

	str += "   ALLOC LOCATION :";
	str += int_to_string(alloc_node->get_time_stamp());

	return str;
}

/***********
 * AllocContextController
 */

AllocContextController::~AllocContextController() {

	map<MemNode*, AllocContext*>::iterator it = alloccs.begin();

	for (; it != alloccs.end(); it++) {
		delete (*it).second;
	}
}



void AllocContextController::collect_alloc_context(AllocContext* allctx,
		AccessPath* alloc_deref_ap) {
	alloccs[allctx->get_alloc_mem_node()] = allctx;

	lhs_rhs_aps[allctx->get_alloc_lhs_ap()] = alloc_deref_ap;
}

AccessPath* AllocContextController::get_alloc_deref_ap(AccessPath* lhs_ap){
	assert(lhs_ap != NULL);
	assert(lhs_rhs_aps.count(lhs_ap) > 0);
	return lhs_rhs_aps[lhs_ap];
}

sail::FunctionCall* AllocContextController::get_alloc_ins(MemNode* mn) {
	assert(mn != NULL);

	return alloccs[mn]->get_alloc_ins();

}

sail::Variable* AllocContextController::get_alloc_ins_lhs_var(MemNode* mn){
	assert(mn != NULL);
	return  alloccs[mn]->get_alloc_ins()->get_lhs();
}


AccessPath* AllocContextController::get_alloc_lhs_ap(MemNode* mn){
	assert(mn != NULL);
	assert(alloccs.count(mn) > 0);
	assert(alloccs[mn] != NULL);

	return alloccs[mn]->get_alloc_lhs_ap();
}



bool AllocContextController::has_node(MemNode* mn){
	return alloccs.count(mn) > 0;
}

map<MemNode*, AllocContext*>& AllocContextController::get_allocs_map(){
	return this->alloccs;
}

const string AllocContextController::to_string() const{

	string str;

	map<MemNode*, AllocContext*>::const_iterator it = alloccs.begin();
	for(; it != alloccs.end(); it++){
		str += it->first->to_string() ;
		AllocContext* ac = it->second;
		str += ac->to_string();
		str += "\n";
	}

	return str;
}
