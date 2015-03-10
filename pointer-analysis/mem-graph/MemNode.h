#ifndef _MEMORYNODE_H
#define _MEMORYNODE_H
#include "MemEdge.h"
#include "DexToSail.h"
#include "access-path.h"
#include  <climits>
#include "InsMemAnalysis.h"

class target{
	set<sail::Constant*> const_target;
	//deal with the case the check var is argument, it contain a unknown value
	set<AccessPath*> others;
public:

	bool has_other() const{
		return (others.size() > 0);
	}

	void add_to_target_as_const(sail::Constant* c){
			const_target.insert(c);
	}

	bool has_constant() const{
		return (const_target.size() > 0);
	}

	void add_to_target_as_ap(AccessPath* ap){
			others.insert(ap);
	}
	bool has_no_value() const{
		return ((const_target.size() == 0)&&(others.size() == 0));
	}
	const unsigned int size_of_const() const{
		return const_target.size();
	}
	sail::Constant* get_first_const(){
		return *(const_target.begin());
	}
	set<sail::Constant*>& get_constants(){
		return this->const_target;
	}

	set<AccessPath*>& get_others(){
		return this->others;
	}

	void clear(){
		const_target.clear();
		others.clear();
	}

	string to_string() const{
		string str;
		if(has_no_value()){
			str = "do not contain any value, EMPTY! " ;
			return str;
		}

		str = "contain constant : ";
		set<sail::Constant*>::iterator it = const_target.begin();
		for(; it != const_target.end(); it++){
			str += int_to_string((*it)->get_integer_value());
		}
		str += "\n";
		if(has_other()){
			str += "others :: ";
			set<AccessPath*>::iterator it = others.begin();
			for(; it != others.end(); it++){
				str += (*it)->to_string();
			}
		}
		return str;
	}

};



class MemEdge;
class MemGraph;

class MemNode{
	friend class MemGraph;
	friend class MemEdge;
private:

	AccessPath* _representative;
	map<int, AccessPath*>* _offset_to_ap;
	map<int, set<MemEdge*>*>* _succs;
	map<int, set<MemEdge*>*>* _preds;

	/*****
	 * is this a node that coming from a _default_edge; look backward;
	 */
	MemEdge* _default_edge;
	bool _default_node;
	unsigned int _node_time_stamp;
	bool _assert_node;

	bool _widening_node;

	/******
	 * a map of (next-level) default_node that coming from this node(key is offset);
	 * look forward e.g variable type has a default deref node;
	 */
	bool _has_default_nodes;

	map<int, MemNode*>* _default_nodes;

public:
	map<int, AccessPath*>* get_offset_2_ap_map() const ;
	map<int, set<MemEdge*>*>* get_succs() const;
	map<int, set<MemEdge*>*>* get_preds() const;

	set<MemEdge*>* get_succ_from_offset(const int offset);
	AccessPath* get_access_path(const int  offset) const;
	const int get_time_stamp() const {return _node_time_stamp;}
	il::type* get_type() const;

	MemNode* get_inner_ap_node()const;

	AccessPath* get_inner_ap()const;

	const string to_string() const;
	const string to_string(bool  pp) const ;
	string escape_dotty_string(string s) const ;
	const string to_dotty() const;

	void collect_target_value(target& t);

	~MemNode();
	MemNode();
	MemNode(const MemNode& other);
	bool is_terminal() const;
	bool is_default_node() const {return this->_default_node;}
	AccessPath* get_representative() const;
	set<MemNode*>* get_targets_from_offset(int  offset) const;
	const bool has_default_nodes() const;
	map<int, MemNode*>* get_default_node() const;

private:
	MemNode(UnmodeledValue* representative);
	MemNode(unsigned int timestamp, AccessPath* representative);

	inline void init_mem_node();
	void one_field_node_init();

	void set_as_default_node() {_default_node = true;}

	void set_default_edge(MemEdge* e);
	MemEdge* get_default_edge();

	void add_mem_node_info(int  offset, AccessPath* ap);

	void insert_to_default_map(int offset, MemNode* mn);


	void set_as_assert_node() ;
	const bool is_assert_node()const;
	void add_2_succs(const int  offset,  MemEdge*  edge);

	void add_2_preds (const int  offset, MemEdge*  edge);
};
#endif
