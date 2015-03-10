#include "MemNode.h"
#define CHECK_NODE_NO true
#define UNKNOWN_VALUE LONG_MAX

MemNode::MemNode(UnmodeledValue* representative){
	_widening_node = true;
	//_representative = UnmodeledValue::make(cur_type, ut, dt);
	_representative = representative;
	_offset_to_ap = new map<int, AccessPath*>();
	_succs = new map<int, set<MemEdge*>*> ();
	_default_node = false;
	cout << " <<<<<<<<<,, making a new UnmodeledValue widening node " << this->to_string() << endl;
	_assert_node = false;
	_default_edge = NULL;


	_has_default_nodes = false;
	_default_nodes = NULL;

}


MemNode::MemNode(unsigned int timestamp, AccessPath* representative){
	_widening_node = false;
	_node_time_stamp = timestamp;
	_representative = representative;
	_offset_to_ap = new map<int, AccessPath* >();
	_succs = new map<int, set<MemEdge*>*> ();
	_preds = new map<int, set<MemEdge*>*> ();
	_default_node = false;
	_default_edge = NULL;
	if(0)
		cout << " <<<<<<<<< making a new node ::::: >>>>>>>>> " <<
			this->to_string() << endl;
	_assert_node = false;

	/******
	 * add for default node;
	 */
	_has_default_nodes = false;
	_default_nodes = NULL;

}


MemNode::MemNode(const MemNode& other) {
	_node_time_stamp = other._node_time_stamp;
	_representative = other._representative;
	_offset_to_ap = other._offset_to_ap;
	_succs = other._succs;
	_preds = other._preds;
	_default_node = other._default_node;
	_widening_node = other._widening_node;
}


inline void MemNode::init_mem_node(){
	assert(_representative != NULL);
	(*_offset_to_ap)[0] = _representative;
}



MemNode::~MemNode(){

	cout << "deleting mem node " << this->_representative->to_string() << endl;
	if(this->_representative->get_ap_type() == AP_UNMODELED)
		return;
	if(this->_representative->get_ap_type() == AP_DEREF){
		if(_representative->get_inner()->get_ap_type() == AP_UNMODELED)
			return;
	}

	delete _offset_to_ap;
	delete _succs;
	delete _preds;
}


bool MemNode::is_terminal() const{
	return (_succs->size() == 0);
}


AccessPath* MemNode::get_representative() const {
	assert(this != NULL);
	return this->_representative;
}



map<int, AccessPath*>* MemNode::get_offset_2_ap_map() const {
	return this->_offset_to_ap;
}



map<int, set<MemEdge*>*>* MemNode::get_succs() const{
	return this->_succs;

}



map<int, set<MemEdge*>*>* MemNode::get_preds()  const{
	return this->_preds;
}

set<MemEdge*>* MemNode::get_succ_from_offset(const int offset){
	assert(_succs->count(offset) >0);
	return (*_succs)[offset];
}


il::type* MemNode::get_type() const{
	return this->_representative->get_type();
}



MemNode* MemNode::get_inner_ap_node()const{

	assert(_preds->size() == 1);

	assert(_preds->count(0) > 0 );
	set<MemEdge*>* edges = (*_preds)[0];
	for(set<MemEdge*>::iterator it = edges->begin(); it != edges->end(); it++){
		MemEdge* edge = *it;

		if(edge->is_default_edge()){
			return edge->get_source_node();
		}
	}
	assert(false);

}

AccessPath* MemNode::get_inner_ap()const{
	assert(_representative->get_ap_type() == AP_DEREF);
	AccessPath* ap = _representative->get_inner();
	return ap;
}


void MemNode::set_default_edge(MemEdge* e){
	assert(e != NULL);
	_default_edge = e;
}

MemEdge* MemNode::get_default_edge(){
	if(_default_node)
		assert(_default_edge != NULL);

	return _default_edge;

}

void MemNode::add_mem_node_info(int  offset, AccessPath*  ap){
	assert(_offset_to_ap != NULL);
	if(_offset_to_ap->count(offset) > 0) //find
		return;
	(*_offset_to_ap)[offset] = ap;
}



void MemNode::add_2_succs(const int  offset, MemEdge*  edge){
	assert(_succs != NULL);
	if(_succs->count(offset) <= 0) //set built
		(*_succs)[offset] = new set <MemEdge*>();

	assert((*_succs)[offset]!= NULL);
	(*_succs)[offset]->insert(edge);
	if(0)
		cout << "YYYYYYYYYYYYYYYYYYYYYYY inserting edge "
			<< edge->to_string() << "to " << this->_representative->to_string()<< endl;

}


void MemNode::insert_to_default_map(int offset, MemNode* mn){
	assert(mn != NULL);

	if(this->_default_nodes ==  NULL)
		_default_nodes = new map<int, MemNode*>();

	if(this->_default_nodes->count(offset) == 0)
		(*_default_nodes)[offset] = mn;

	this->_has_default_nodes = true;
}




void MemNode::add_2_preds (const int  offset, MemEdge*  edge){
	assert(_preds != NULL);
	if(_preds->count(offset) <= 0)
		(*_preds)[offset] = new set <MemEdge*>();
	assert((*_preds)[offset]!= NULL);
	(*_preds)[offset]->insert(edge);
}




AccessPath* MemNode::get_access_path(const int  offset) const{

	if(_offset_to_ap->count(offset) <= 0){
		cerr << "Node is " << to_string() << endl;
		cerr << "want to find accesspath in offset    " <<offset  <<  endl;
	}

	assert(_offset_to_ap->count(offset) >0 );
	if(_offset_to_ap->count(offset) > 0)
		return (*_offset_to_ap)[offset];
	//return NULL;
}




//succes MemNode of of some offset
set<MemNode*>* MemNode::get_targets_from_offset(int  offset) const{
	set<MemNode*>* target_nodes = new set <MemNode*> ();
	assert(this->_succs != NULL);
	//cout << "offset is " << offset << endl;



	//FOR TESTINT USAGE
	if(this->_succs->count(offset) <= 0){
		return target_nodes;
	}

	set<MemEdge*>* succ_edges = (*this->_succs)[offset];
	set<MemEdge*>::iterator it = succ_edges->begin();
	for(; it != succ_edges->end(); it ++){
		MemNode* temp = (*it)->get_target_node();
		target_nodes->insert(temp);
	}
	return target_nodes;
}


const bool MemNode::has_default_nodes() const{
	return this->_has_default_nodes;
}

map<int, MemNode*>* MemNode::get_default_node() const{
	return this->_default_nodes;
}

void MemNode::one_field_node_init() { //only process one level, no recursive
	if ((_representative->get_ap_type() == AP_CONSTANT)
			|| (_representative->get_ap_type() == AP_VARIABLE)) { //first level
		init_mem_node();
	}else if(_representative->get_ap_type() == AP_ALLOC){
		init_mem_node();
	}else if(_representative->get_ap_type() == AP_STRING){
		init_mem_node();
	}else if(_representative->get_ap_type() == AP_ARITHMETIC){
		init_mem_node();
	}else if(_representative->get_ap_type() == AP_FUNCTION){
		init_mem_node();
	}else if(_representative->get_ap_type() == AP_UNMODELED){
		init_mem_node();
	}
	else { //second or third level
		if (0) {
			cout << "representative "
					<< _representative->to_string() << endl;
			cout << "representative ap type "
					<< this->_representative->get_ap_type() << endl;
		}

		assert(this->_representative->get_ap_type() == AP_DEREF);

		if(_representative->get_inner()->get_ap_type() == AP_ALLOC){
			return;
		}else if (_representative->get_inner()->get_ap_type() == AP_CONSTANT) //second level
			init_mem_node();
		else if(_representative->get_inner()->get_ap_type() == AP_ARITHMETIC){
			init_mem_node();
		}else if(_representative->get_inner()->get_ap_type() == AP_FUNCTION){
			init_mem_node();
		}else if(_representative->get_inner()->get_ap_type() == AP_UNMODELED)
			init_mem_node();
		else if (_representative->get_inner()->get_ap_type()
				== AP_VARIABLE) {//second level
			Variable* pre_ap =
					static_cast<Variable*> (_representative->get_inner());

			sail::Variable* sail_var = pre_ap->get_original_var();
			if (!(sail_var->get_type()->is_pointer_type())) { //not pointer type, so base_type, init it
				init_mem_node();
			}
		}else  { //third level
			init_mem_node();
		}
	}
}

////////////////////////////////////


void MemNode::collect_target_value(target& t) {
	assert(this->_representative->get_ap_type() == AP_VARIABLE);
	map<int, set<MemEdge*>*>* succs = get_succs();
	assert(succs != NULL);
	//cout << _representative->to_string() << endl;
	//hzhu changed 6/12
	//cout <<"succs size is " << succs->size() << endl;
	//assert(succs->size() == 1);
	if(succs->size() == 1){

	set<MemEdge*>* succ_edges = succs->begin()->second;
	for (set<MemEdge*>::iterator it = succ_edges->begin();
			it != succ_edges->end(); it++) {

		AccessPath* t_ap = (*it)->get_target_node()->get_representative();
		assert(t_ap->get_ap_type() == AP_DEREF);
		Deref* deref = static_cast<Deref*>(t_ap);
		AccessPath* inner_ap = deref->get_inner();

		if (inner_ap->get_ap_type() == AP_CONSTANT) {

			long int value = ((ConstantValue*) inner_ap)->get_constant();
			sail::Constant* cons = new sail::Constant(value, true, 32);
			t.add_to_target_as_const(cons);
		} else {

			cout << "inner_ap->get_ap_type()" << inner_ap->get_ap_type() << endl;
			assert(
					(inner_ap->get_ap_type() == AP_VARIABLE) ||(inner_ap->get_ap_type() == AP_FIELD) ||(inner_ap->get_ap_type() == AP_ARITHMETIC));
			t.add_to_target_as_ap(inner_ap);
		}
	}
}
}



void MemNode::set_as_assert_node(){
	this->_assert_node = true;
}



const bool MemNode::is_assert_node() const{
	bool flag = this->_assert_node;
	return flag;
}



const string MemNode::to_string() const
{


	string res;
	if(!_widening_node)
		res = "Node NO. :: " + int_to_string(this->_node_time_stamp);

	map<int, AccessPath*>::iterator it = _offset_to_ap->begin();

	//to adjust print format
	int i=0;
	int size = _offset_to_ap->size();

	for(; it!= _offset_to_ap->end(); it++, i++)
	{
		int offset = it->first;
		AccessPath* ap = it->second;
		res += "[" + int_to_string(offset) + "->" + ap->to_string() + "]";
		if(i != size -1) res += ",";
	}
	return res;
}



const string MemNode::to_string(bool  pp) const {

	if (!pp)
		return to_string();

	return _representative->to_string();
}


//for escape, need to process it with "\\"
string MemNode::escape_dotty_string(string s) const {
	string res;
	for (unsigned int i = 0; i < s.size(); i++) {
		if (s[i] == '>') {
			res += "\\>";
			continue;
		}
		if (s[i] == '<') {
			res += "\\<";
			continue;
		}
		if (s[i] == '|') {
			res += "\\|";
			continue;
		}
		if (s[i] == '{') {
			res += "\\{";
			continue;
		}
		if (s[i] == '}') {
			res += "\\}";
			continue;
		}
		res += s[i];
	}
	return res;
}



const string MemNode::to_dotty() const {

	string s = "{{";
	map<int, AccessPath*>::iterator it = _offset_to_ap->begin();
	int i = 0;
	for (; it != _offset_to_ap->end(); it++, i++) {
		s += "{";
		if (it->first >= 0)
			s += "<off_e" + int_to_string(it->first) + ">";
		else
			s += "<off_e_minus" + int_to_string(-it->first) + ">";
		s += int_to_string(it->first);
		s += "}";
		if (i < (int) _offset_to_ap->size() - 1)
			s += "|";
	}
	s += "}|{";
	//cout << "MemNode::to_dotty check 1.1 " << endl;
	i = 0;
	it = _offset_to_ap->begin();

	//cout << "size of _offset_to_ap is " << _offset_to_ap->size() << endl;
	for (; it != _offset_to_ap->end(); it++, i++) {
		//cout << " ===> index is " << i << endl;
		s += "{";
		if (it->first >= 0)
			s += "<off_s" + int_to_string(it->first) + ">";
		else
			s += "<off_s_minus" + int_to_string(-it->first) + ">";
		s += escape_dotty_string(it->second->to_string() + ":" +
				it->second->get_type()->to_string());
		s += "}";
		if (i < (int) _offset_to_ap->size() - 1)
			s += "|";
	}
	//cout << "MemNode::to_dotty check 1.2 " << endl;
	//added by haiyan for number of node
	if(CHECK_NODE_NO)
		s += "}|{{[NO." + int_to_string(this->_node_time_stamp) + "]}}";
	//added by haiyan
	s += "}}";

	return s;
}
