#ifndef _SIDE_NODE_H_
#define _SIDE_NODE_H_


#include "sail.h"
#include "type.h"
#include "MemNode.h"
#include "access-path.h"
#include <unordered_set>
#include "MtdInstsProcessor.h"

#define INVALID_OPCODE -3
#define IMPOSSIBLE_OFFSET  -200


/**************
 * representative ap and its reachable nodes;
 */
struct node_set {
	AccessPath* var; //ap(representative) of start node
	set<MemNode*>* nodes;

	node_set(){
		var = NULL;
		nodes = NULL;
	}

	bool operator==(const node_set& _other) const {
		return (var == _other.var);
	}

	bool operator < (const node_set& _other) const{
		return (this->var < _other.var);
	}
};


struct variable_info {
	sail::Variable* var;
	string fn;
	il::type* ft;

	variable_info(){
		var = NULL;
		ft = NULL;
	}


	void operator = (const variable_info& other){
		this->var = other.var;
		this->fn = other.fn;
		this->ft = other.ft;
	}

	const string to_string() const{
		string str;
		if(var != NULL)
			str += var->to_string();
		if(ft != NULL){
			str += fn;
			str += ft->to_string();
		}
		return str;

	}
};

struct av_target {
	unsigned int max_aps_size;
	//representative aps
	set<AccessPath*> av_target;

	string to_string() {
		string str;

		set<AccessPath*>::iterator it = av_target.begin();
		for (; it != av_target.end(); it++) {
			str += (*it)->to_string();
			//			if (it != (av_target.end())--) {
			//				str += "\n";
			//			}
		}
		str += "\t";
		str += "(max ap size : ";
		str += int_to_string(max_aps_size);
		//str += "\n";
		str += ")";
		return str;
	}

	void insert_ap_target(AccessPath* ap) {
		this->av_target.insert(ap);
	}


	void delete_ap_target(AccessPath* ap) {
		this->av_target.erase(ap);
	}

};

////////////////////////////////////
////////////////////////////////////

struct ap_with_operator {
	AccessPath* ap;
	long int op;

	ap_with_operator() {
		op = INVALID_OPCODE;
	}

	string to_string() const;

	bool operator ==(ap_with_operator& other) const;
};



struct ins_two_side {
	struct instruction_side* lhs;
	struct instruction_side* rhs;
};



struct one_side {
	sail::Symbol* var;
	int offset;

	const string to_string() const {
		string str;
		str += "one_side : ";
		str += var->to_string();
		str += "[";
		str += int_to_string(offset);
		str += "]";
		return str;
	}

	one_side(sail::Symbol* var){
		this->var = var;
		this->offset = IMPOSSIBLE_OFFSET;

	}

	one_side(sail::Symbol* var, int offset) {
		this->var = var;
		this->offset = offset;
	}

	bool operator < (const one_side& other) const{

		if(this->var == other.var)
			return this->offset < other.offset;

		return this->var < other.var;

	}


	bool operator== (const  one_side& other) const{
		if(this->var != other.var)
			return false;

		if(this->offset != other.offset)
			return false;

		return true;
	}

};



struct instruction_side {

	sail::Symbol* var;
	int offset;
	bool lhs;

	instruction_side* other_side;

	instruction_side() {
	}

	instruction_side(sail::Instruction* ins, bool lhs) {
		this->lhs = lhs;
		offset = IMPOSSIBLE_OFFSET;
		switch (ins->get_instruction_id()) {
		case sail::ASSIGNMENT: {
			sail::Assignment* assi = static_cast<sail::Assignment*>(ins);
			if (lhs) {
				var = assi->get_lhs();
			} else {
				var = assi->get_rhs();
			}
		}
			break;

		case sail::LOAD: {

			sail::Load* l = static_cast<sail::Load*>(ins);
			if (lhs) {
				var = l->get_lhs();
			} else {
				var = l->get_rhs();
				offset = l->get_offset();
			}
		}
			break;

		case sail::STORE: {
			sail::Store* s = static_cast<sail::Store*>(ins);

			if (lhs) {
				var = s->get_lhs();
				offset = s->get_offset();
			} else {
				var = s->get_rhs();
			}
		}
			break;
		default:
			assert(false);

		}
	}

	const string basic_info_to_string() const {
		string str;

		str += var->to_string();

		str += "Off :: ";
		str += int_to_string(offset);

		str += "\n";
		return str;
	}

	const string to_string() const {
		string str;

		str += basic_info_to_string();

		str += this->other_side->basic_info_to_string();

		return str;

	}

};

//point to same rhs;
struct side_ele {
	instruction_side* ins_side;

	bool is_return_ins;

	sail::Instruction* consist_ins;

	side_ele() {
	}

	side_ele(instruction_side* instruction_side, bool is_return,
			sail::Instruction* ins) {

		ins_side = instruction_side;
		is_return_ins = is_return;
		consist_ins = ins;
	}

	const string half_side_to_string() const {

		return this->ins_side->basic_info_to_string();
	}

	const string to_string() const {
		string str;
		str += ins_side->to_string();

		if (is_return_ins) {

			str += " IS return ins";
			str += ";  ";
		}

		str += consist_ins->to_string();
		return str;

	}
};


class SideNode;

namespace std {

template <>
struct hash<SideNode*> {
        size_t operator() (const SideNode* const & x) const;
};

struct SideNode_eq
{
  bool operator()(const SideNode* l1, const SideNode* l2) const;
};

}


//this one are not used here !!!
#ifdef TERMINAL_SIDES_OPEN

class MtdInstsProcessor;

class SideNode{
private:
	one_side* side;
	//meaning side is lhs in instruction (store all the lhs);
	map<SideNode*, sail::Instruction*> point_to;

	//meaning side is rhs in instruction (store all the lhs);
	set<SideNode*> pointed_by;

	size_t hash_c;


	SideNode(one_side* side);
	~SideNode();

public:
	static unordered_set<SideNode*, std::hash<SideNode*>, SideNode_eq> sidenodes;
	static set<one_side*> unique_sides;

	//check the sidenodes and see if already inside it

	static SideNode* make(one_side* side);

	/*****
	 * used to unique SideNode based on public member sidenodes;
	 */
	static SideNode* get_side_node(SideNode* side);

	bool operator==(const SideNode& __other);

	/*****
	 * get side element;
	 */
	one_side* get_one_side() const ;

	void add_to_point_to_container(SideNode* sn, sail::Instruction* inst);
	void add_to_pointed_by_container(SideNode* sn);


	//used to return the hash_c;
	inline size_t hash_code()
	{
			return hash_c;
	}

	//used to generate hash_c, called in the constructor;
	void compute_hash_code();

	const bool has_point_to() const;
	const string to_string() const;

	static void print_sidenodes();
	static void print_unique_sides();

};
#endif

#endif
