#include "SideNode.h"


/*******************************
 * ap_with_operator struct
 *****************************/

string ap_with_operator::to_string() const {
	string str;
	str = "(";
	str += operator_to_string(op);
	str += ") ";
	str += ap->to_string();
	return str;
}

bool ap_with_operator::operator ==(ap_with_operator& other) const {
	bool flag = false;
	if (ap == other.ap) {
		if (0) {
			cout << "this op is " << op << " " << operator_to_string(op)
					<< endl;
			cout << " other op is " << other.op << " "
					<< operator_to_string(other.op) << endl;
		}
		if (op == INVALID_OPCODE) {
			if (other.op == INVALID_OPCODE) {
				flag = true;
			} else if (reflexible_operator(other.op)) {
				flag = true;
			}
		} else if (reflexible_operator(op)) {
			if (other.op == INVALID_OPCODE) {
				flag = true;
			}
			if (reflexible_operator(other.op)) {
				flag = true;
			}
		} else {
			flag = (op == other.op);
		}
	}
	if(flag)
		cout << "find " << flag << endl;
	return flag;
}


#ifdef TERMINAL_SIDES_OPEN
unordered_set<SideNode*, std::hash<SideNode*>, SideNode_eq> SideNode::sidenodes;
set<one_side*> SideNode::unique_sides;

SideNode* SideNode::make(one_side* side){


	//if found, find the iterator,then delete itself;
	if(unique_sides.count(side) > 0){
		auto it = unique_sides.find(side);
		delete side;
		side = *it;
	}
	else
		unique_sides.insert(side);



	assert(side != NULL);

	SideNode* sn = new SideNode(side);

	//this will set hash_c;
	assert(sn != NULL);
	sn = SideNode::get_side_node(sn);
	return sn;

}


SideNode* SideNode::get_side_node(SideNode* side){

	auto it = sidenodes.find(side);

	//didn't find;
	if(it == sidenodes.end()){
		sidenodes.insert(side);
			return side;
	}

	// find;
	if(*it != side){
		cout << " == collision of hash_function " << endl;
		cout << " == hash_code value ==>  " << side->hash_code() << endl;
	}

	//free space;
	delete side;
	return *it;
}



bool SideNode::operator==(const SideNode& __other){
	return *(this->side) == *(__other.side);
}




SideNode::SideNode(one_side* side){
	this->side = side;
	//this will set hash_c;
	//this->compute_hash_code();

}


SideNode::~SideNode(){

}



one_side* SideNode::get_one_side() const {
	return side;
}

void SideNode::add_to_pointed_by_container(SideNode* lsn){
	this->pointed_by.insert(lsn);

}


void SideNode::add_to_point_to_container(SideNode* rsn, sail::Instruction* inst){

	this->point_to[rsn] = inst;

}


void SideNode::compute_hash_code()
{
	int hash=7;

	string name = side->var->to_string();
	for (int i=0; i < (int)name.length(); i++) {
		hash = hash*31+ name.at(i);
	}
	hash_c = hash;
}


const bool SideNode::has_point_to() const{
	return (this->point_to.size() != 0);
}

const string SideNode::to_string() const{
	string str;

	str += side->to_string();
	str += "\n";


	str += "		As lhs, point to  ";
	if(point_to.size() == 0)
		str += "EMPTY! ";
	else {
		for (auto it = point_to.begin(); it != point_to.end(); it++) {
			str += "\n";
			str += it->first->side->to_string();

			str += " in instruction : ";
			str += it->second->to_string();
			str += "\n";
		}
	}
	str += "\n";
	str += "		###################### " ;
	str += "\n";
	str += "		As rhs, pointed by  " ;

	if(pointed_by.size() == 0){
		str += "EMPTY !";
	} else {
		for (auto it = pointed_by.begin(); it != pointed_by.end(); it++) {
			str += "\n";
			str += "\t";
			str += (*it)->side->to_string();
			str += "\n";
		}
	}
	str += "		------------------- the end!";

	return str;
}



void SideNode::print_sidenodes()
{
	cout << "		inside Print SideNodes ! " << endl;
	auto it = sidenodes.begin();
	for(; it != sidenodes.end() ; it ++){
		cout <<"		SideNode ==>" <<(*it)->to_string() << endl;
	}

	cout << " 		out of Print SideNodes- - - -  - - --  - " << endl;
}


void SideNode::print_unique_sides()
{
	cout << "Print one_sides ! " << endl;
	auto it = unique_sides.begin();

	for(; it != unique_sides.end(); it ++){
		cout <<"one_side =>" << (*it)->to_string() << endl;
	}
	cout << " - - - -  - - --  - " << endl;
}


namespace std {

/*******
 * return hash_value;
 */
size_t hash<SideNode*>::operator() (const SideNode* const & x) const {
	SideNode*& xx = (SideNode*&)x;
	xx->compute_hash_code();
	return xx->hash_code();
}

bool SideNode_eq::operator()(const SideNode* sn1, const SideNode* sn2) const
{

	if(sn1->get_one_side()->to_string() == sn2->get_one_side()->to_string())
		return true;
	return false;
}

}
#endif
