#include "MemEdge.h"


MemEdge::MemEdge(int count, MemNode* source, MemNode* target, int s_off,
		int t_off) :
		_source(source), _target(target),
		_source_offset(s_off), _target_offset(t_off) {
	_time_stamp = count;
	_default_edge = false;
	if(0)
		cout << "HHHHHHHHHH Creating new edge HHHHHHHHHHHHHH  NO." <<
		_time_stamp  << "  "<< to_string() << endl;

	_init_edge = false;
}


MemEdge::MemEdge(const MemEdge& other) {
	_source_offset = other._source_offset;
	_target_offset = other._target_offset;
	_default_edge = other._default_edge;
	_source = other._source;
	_target = other._target;
	_time_stamp = other._time_stamp;

	_init_edge = other._init_edge;
}

MemEdge::~MemEdge() {
}



MemNode* MemEdge::get_source_node() const {
	return _source;
}



MemNode* MemEdge::get_target_node() const {
	return _target;
}



AccessPath* MemEdge::get_source_ap(int  offset) const{
	return _source->get_access_path(offset);
}



AccessPath* MemEdge::get_target_ap(int  offset) const {
	return _target->get_access_path(offset);
}


int MemEdge::get_source_offset()  const {
	return _source_offset;
}



int MemEdge::get_target_offset() const {
	return _target_offset;
}



string MemEdge::to_string() const
{
	string res = "(" + _source->to_string() + ", "+
		int_to_string(_source_offset) + ") -> ";
	res += "(" + _target->to_string() + ", "+
	int_to_string(_target_offset) + ")";
	return res;
}



void MemEdge::add_edge_to_nodes(){
	this->get_source_node()->add_2_succs(this->get_source_offset(), this);
	this->get_target_node()->add_2_preds(this->get_target_offset(),this);

}
