#ifndef _CONSTRAINT_H
#define _CONSTRAINT_H

#include "ConstraintItem.h"
#include <set>
using namespace std;


class Constraints{
private:
	set<ConstraintItem*> constraints;
public:
	Constraints();
	~Constraints();
	void add_to_constraints(ConstraintItem* con);
	void erase_constrant(ConstraintItem* con);
	bool has_constraints();
	bool contain_symbol(sail::Symbol* s);
	string to_string();
	set<ConstraintItem*>& get_constraints_content();

};


#endif
