#ifndef IL_SWITCH_STATEMENT_H_
#define IL_SWITCH_STATEMENT_H_

#include "control_statement.h"

#include <vector>
#include <map>
#include <set>

using namespace std;

namespace il
{
	class expression;
	class case_label;

/**
 * \brief The switch construct
 */
class switch_statement : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
    	ar & cond;
    	ar & body;
    	ar & case_labels;
    }
private:
	expression* cond;
	statement* body; /* For empty bodies, this isn't a block, so we'll
					  * keep it a statement. */
	vector<case_label*>* case_labels;

	/*
	 * The set of all contiguous ranges covered by the
	 * non-default cases of this switch statement.
	 * We use this to determine the canonical representation
	 * of the condition for the default case.
	 */
	vector<pair<long, long> > case_label_ranges;


	/*
	 * Mapping from each case label to its equivalence class
	 * representative. Two case labels belong to the same equivalence class
	 * if one falls through to the other without any intervening instructions.
	 * Example:
	 * case 1:
	 * case 2:
	 * so something;
	 * Here, case 1 and case 2 are in the same equivalence class.
	 * THESE ARE NOT SERIALIZED.
	 */
	map<case_label*, vector<case_label*>* >* equivalence_classes;

	map<case_label*, vector<pair<long, long> > * > equivalence_class_ranges;

public:
	switch_statement();
	switch_statement(expression* cond, statement* body,
			vector<case_label*>* case_labels,
			map<case_label*, vector<case_label*>* >* representatives,
			location loc);
	expression* get_cond();
	statement* get_body();
	vector<case_label*>* get_case_labels();


	/*
	 * Returns coalesced contigous ranges. E.g. if we have two case
	 * labels case 3: and case 4...7:, this would return
	 * a singe range case 3 ... 7
	 */
	vector<pair<long, long> > & get_case_ranges();

	/*
	 * Returns the ranges for each equivalence class EXCEPT the one
	 * containing the default label.
	 */
	map<case_label*, vector<pair<long, long> > * > &
			get_equivalence_class_ranges();
	virtual ~switch_statement();
	virtual void print();
	virtual string to_string() const;
	map<case_label*, vector<case_label*>* >* get_label_equivalence_classes();

private:

	/*
	 * Helper to compute case_label_ranges
	 */
	void coalesce_labels(vector<case_label*> * labels,
			vector<pair<long, long> > & ranges,
			set<case_label*> * default_eq_class);

};

}

#endif /*SWITCH_STATEMENT_H_*/
