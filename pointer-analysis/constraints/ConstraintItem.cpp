#include "ConstraintItem.h"
ConstraintItem::ConstraintItem() {
	constraint = new set<constraint_ele*>();
}


ConstraintItem::~ConstraintItem() {

}



void ConstraintItem::add_to_contraint(constraint_ele* ele) {
	this->constraint->insert(ele);
}



set<constraint_ele*>*& ConstraintItem::get_content() {
	return this->constraint;
}



int ConstraintItem::get_which(sail::Symbol* s) {
	for (set<constraint_ele*>::iterator it = constraint->begin();
			it != constraint->end(); it++) {
		if ((*it)->get_symbol() == s)
			return (*it)->get_which();
	}
}


long int ConstraintItem::get_value(sail::Symbol* s) {
	for(set<constraint_ele*>::iterator it = constraint->begin();
			it != constraint->end(); it++){
		if((*it)->get_symbol() == s)
			return (*it)->get_value();
	}
	cout << "return impossible long value " << endl;
	return IMPOSSIBLE_VALUE;
}



AccessPath* ConstraintItem::get_ap(sail::Symbol* s) {
	for (set<constraint_ele*>::iterator it = constraint->begin();
			it != constraint->end(); it++) {
		if ((*it)->get_symbol() == s)
			return (*it)->get_ap();
	}
}



void ConstraintItem::set_value(sail::Symbol* s, long int value){
	for(set<constraint_ele*>::iterator it = constraint->begin();
				it != constraint->end(); it++){
		if((*it)->get_symbol() == s){
			(*it)->set_value(value);
			return;
		}
	}
	cout << "can't find symbol ::" << s->to_string() << endl;
	assert(false);
}



bool ConstraintItem::has_two_symbols(sail::Symbol* oper1_s, sail::Symbol* oper2_s){
	bool oper1_s_find = false;
	bool oper2_s_find = false;
	for(set<constraint_ele*>::iterator it = constraint->begin();
				it != constraint->end(); it++){
		if((*it)->get_symbol() == oper1_s)
			oper1_s_find = true;
		if((*it)->get_symbol() == oper2_s)
			oper2_s_find = true;
	}
	return (oper1_s_find && oper2_s_find);
}



bool ConstraintItem::has_symbol(sail::Symbol* s) {
	for (set<constraint_ele*>::iterator it = constraint->begin();
			it != constraint->end(); it++) {
		if ((*it)->get_symbol() == s)
			return true;
	}
	return false;
}




string ConstraintItem::to_string(){
	string str;
	for(set<constraint_ele*>::iterator it = constraint->begin();
				it != constraint->end(); it++){
		str += (*it)->to_string();
		if(it != --(constraint->end())){
			str += "\t";
		}
	}
	return str;
}
