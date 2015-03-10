/*
 * Branch.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *   We treat every branch as a multi-branch. This allows us to treat
 *   if-then-else and switch statements uniformly. In addition, we maintain
 *   the invariant that all conditions in a multibranch statement are
 *   muturally exclusive and add up to true.
 *
 *
 */

#ifndef BRANCH_H_
#define BRANCH_H_

#include "Instruction.h"

#include<string>
#include <vector>
using namespace std;

namespace il{class node;}

namespace sail {
class Variable;
class Label;
class Symbol;

/**
 * \brief A branch instruction is of the form s1!=0 : l1, s2!=0 : l_2 , ... s_k!=0:l_k
 */
/**
 * Any s_i !=0 and s_j !=0 are mutually exclusive, and the disjunction
 * s_1!=0 | ....|s_k!=0 is always true.
 * The (s_i, l_i) pairs are represented by the vector called targets, i.e.
 * the result of get_targets().
 */
class Branch: public Instruction  {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & targets;
        ar & is_if;
    }

private:
	vector<pair<Symbol*, Label*> > *targets;
	bool is_if;

public:
	/*
	 * Constructor for if
	 */
	Branch(Symbol* if_var, Symbol* else_var, Label* then_label, Label* else_label,
			il::node* original);

	/*
	 * Constructor for switch
	 */
	Branch(vector<pair<Symbol*, Label*> > *targets,
				il::node* original);
	Branch(){targets = NULL;};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;

	/**
	 * @return Is this branch associated with an if rather than a switch statement?
	 * If so, the convenience methods provided below for if statements
	 * can be called.
	 */
	bool is_if_statement();

	/**
	 * @return a vector of (s_i, l_i) pairs representing the condition and
	 * the label to jump to if the condition s_i != 0 holds.
	 */
	vector<pair<Symbol*, Label*> > * get_targets();

	/**
	 * Convenience method for branches corresponding to if statements
	 * @return the label associated with the true branch.
	 */
	Label* get_then_label();

	/**
	 * Convenience method for branches corresponding to if statements
	 * @return the label associated with the false branch.
	 */
	Label* get_else_label();

	/**
	 * Convenience method for branches corresponding to if statements
	 * @return the condition for the true branch
	 */
	Symbol* get_if_condition();

	/**
	 * Convenience method for branches corresponding to if statements
	 * @return the condition for the false branch
	 */
	Symbol* get_else_condition();
	virtual bool is_removable();
	virtual ~Branch();
};

}

#endif /* BRANCH_H_ */
