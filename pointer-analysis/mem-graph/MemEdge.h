#ifndef _MEMEDGE_H
#define _MEMEDGE_H
#include "DexToSail.h"
#include "access-path.h"
#include "MemNode.h"
class MemNode;


class MemEdge {
	friend class MemGraph;

private:
	MemNode* _source;
	MemNode* _target;
	int _source_offset;
	int _target_offset;
	int _time_stamp;
	bool _default_edge;
	bool _init_edge;
public:

	~MemEdge();
	MemEdge(const MemEdge& other);
	MemNode* get_source_node()const;
	MemNode* get_target_node()const;
	int get_source_offset()const;
	int get_target_offset()const;
	AccessPath* get_source_ap(int offset) const;
	AccessPath* get_target_ap(int offset)const;

	bool is_default_edge()const {return this->_default_edge;}
	bool is_init_edge() const {return this->_init_edge;}
	string to_string() const;

	int get_time_stamp() const {return _time_stamp;}
private:
	MemEdge(int counter, MemNode* source, MemNode* target, int s_off,
				int t_off);
	void add_edge_to_nodes();
	void set_as_default_edge(){this->_default_edge = true;}
	void set_as_init_edge(){this->_init_edge = true;}
};
#endif
