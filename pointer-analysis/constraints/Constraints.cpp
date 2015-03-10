#include "Constraints.h"

Constraints::Constraints() {

}


Constraints::~Constraints() {

}



void Constraints::add_to_constraints(ConstraintItem* con) {
	this->constraints.insert(con);
}



void Constraints::erase_constrant(ConstraintItem* con) {
	assert(constraints.find(con) != constraints.end());
	this->constraints.erase(con);
}



bool Constraints::has_constraints() {
	return (constraints.size() > 0);
}



set<ConstraintItem*>& Constraints::get_constraints_content(){
	return this->constraints;
}



bool Constraints::contain_symbol(sail::Symbol* s){
	if(constraints.size() >0){
		if((*constraints.begin())->has_symbol(s))
			return true;
	}
	return false;
}


string Constraints::to_string() {
	string str;
	for (set<ConstraintItem*>::iterator it = constraints.begin();
			it != constraints.end(); it++) {
		str += "{";
		str += (*it)->to_string();
		str += "}";
		str += "\n";
	}
	return str;
}

