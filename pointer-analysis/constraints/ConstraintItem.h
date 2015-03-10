#ifndef _CONSTRAINTITEM_H
#define _CONSTRAINTITEM_H

#include <set>
#include <iostream>
#include "util.h"
#include <climits>
#include <assert.h>
#include "sail.h"
#include "il.h"
#include "access-path.h"

#define IMPOSSIBLE_VALUE LONG_MIN
#define VALUE_CONTENT 0
#define AP_CONTENT 1

using namespace std;
struct constraint_ele{
	sail::Symbol* s;
	int which;
	union {
		long int value;
		AccessPath* ap;
	}content;

	constraint_ele(sail::Symbol* s, long int value){
		this->s = s;
		content.value = value;
		which = VALUE_CONTENT;
	}

	constraint_ele(sail::Symbol* s, AccessPath* ap){
		this->s = s;
		content.ap = ap;
		which = AP_CONTENT;
	}

	int get_which (){
		return which;
	}

	long int get_value(){
		return content.value;
	}

	AccessPath*& get_ap(){
		return content.ap;
	}
	sail::Symbol* get_symbol(){
		return s;
	}
	void set_value(long int val){
		this->content.value = val;
	}

	string to_string(){
		string str;
		str = "(";
		str += s->to_string();
		str += " -> ";
		if(which == VALUE_CONTENT)
			str += int_to_string(content.value);
		else
			str += content.ap->to_string();
		str += ")";
		return str;
	}
};

class ConstraintItem{
private:
	set<constraint_ele*>* constraint;
public:
	ConstraintItem();
	~ConstraintItem();
	bool has_two_symbols(sail::Symbol* oper1_s, sail::Symbol* oper2_s);
	bool has_symbol(sail::Symbol* oper1_s);
	void add_to_contraint(constraint_ele* ele);
	set<constraint_ele*>*& get_content();
	int get_which(sail::Symbol* s);
	long int get_value(sail::Symbol* s);
	AccessPath* get_ap(sail::Symbol* s);
	void set_value(sail::Symbol* s, long int value);
	string to_string();
};

#endif
