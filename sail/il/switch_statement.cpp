#include "switch_statement.h"
#include "expression.h"
#include "case_label.h"
#include <algorithm>

#define DEBUG false


namespace il
{
switch_statement::switch_statement()
{
	case_labels = NULL;
	equivalence_classes = NULL;
}


switch_statement::switch_statement(expression* cond, statement* body,
		vector<case_label*>* case_labels,
		map<case_label*, vector<case_label*>* >* equivalence_classes, location loc)
{

	if(DEBUG)
	{
		cout << "Making switch statement: " << endl;
		cout << "Cond: " << cond->to_string() << endl;
		cout << "Body: " << body->to_string() << endl;
		for(unsigned int i =0; i<case_labels->size(); i++)
		{
			case_label* cl = (*case_labels)[i];
			cout << "Case label " << i << ": " << cl->to_string() << endl;
		}

		map<case_label*, vector<case_label*>* >::iterator it =
					equivalence_classes->begin();
		for(; it!= equivalence_classes->end(); it++)
		{
			case_label* cl = it->first;
			cout << "Equivalence class of: " << cl->to_string() << endl;
			vector<case_label*>* eq = it->second;
			vector<case_label*>::iterator it2 = eq->begin();
			for( ;it2!= eq->end(); it2++) {
				case_label* cur = *it2;
				cout << "\t " << cur->to_string() << endl;
			}
		}
	}

	this->cond = cond;
	this->body = body;
	this->case_labels = case_labels;
	this->loc = loc;
	this->node_type_id = SWITCH_STMT;
	this->equivalence_classes = equivalence_classes;

	set<case_label*> default_eq_class;


	// compute ranges for every equivalence class
	map<case_label*, vector<case_label*>* >::iterator it;
		for(it = equivalence_classes->begin();
			it != equivalence_classes->end(); it++)
		{
			if(it->first->is_default_label())
			{
				default_eq_class.insert(it->second->begin(),it->second->end());
				continue;
			}
			vector<case_label*> * eq_class = it->second;
			vector<pair<long, long> > * vec = new vector<pair<long, long> >();
			coalesce_labels(eq_class, *vec, NULL);
			equivalence_class_ranges[it->first] = vec;
		}

	//compute all ranges for default condition
	coalesce_labels(case_labels, case_label_ranges, &default_eq_class);

	// For debugging
	/*cout << "LABEL RANGES: " << endl;
	for(int i=0; i<(int) case_label_ranges.size(); i++)
	{
		cout << "\t Range " << i+1 << ": (" << case_label_ranges[i].first <<
			", " << case_label_ranges[i].second << ")" << endl;
	}
	cout << "EQUIVALENCE CLASSES COMING... " << endl;

	map<case_label*, vector<case_label*>* >::iterator it2;
	for(it2 = equivalence_classes->begin();
		it2 != equivalence_classes->end(); it2++)
	{
		cout << "** EQ rep: " << it2->first->to_string() << endl;
		for(int i = 0; i < (int)it2->second->size(); i++)
		{
			cout << "\t" << (*it2->second)[i]->to_string() << endl;
		}
	}
	*/
}



string switch_statement::to_string() const
{
	string res = "switch(" + cond->to_string() + ") \n";
	res += body->to_string();
	return res;
}

void switch_statement::print()
{
	cout << this->to_string() << endl;
}

expression* switch_statement::get_cond()
{
	return cond;
}

statement* switch_statement::get_body()
{
	return body;
}

vector<case_label*>* switch_statement::get_case_labels()
{
	return case_labels;
}

vector<pair<long, long> > & switch_statement::get_case_ranges()
{
	return case_label_ranges;
}

switch_statement::~switch_statement()
{
	delete case_labels;
	delete equivalence_classes;
}

map<case_label*, vector<case_label*>* >*
switch_statement::get_label_equivalence_classes()
{
	return equivalence_classes;
}

map<case_label*, vector<pair<long, long> > * > &
		switch_statement::get_equivalence_class_ranges()
{
	return this->equivalence_class_ranges;
}

// -----------------------------------
void switch_statement::coalesce_labels(vector<case_label*> * labels,
		vector<pair<long, long> > & ranges, set<case_label*> * default_eq_class)
{
	vector<pair<long, long> > initial_ranges;
	for(int i=0; i<(int)labels->size(); i++)
	{
		case_label* cur = (*labels)[i];
		if(default_eq_class != NULL && default_eq_class->count(cur) > 0)
			continue;
		initial_ranges.push_back(
				pair<long,long>(cur->get_low(), cur->get_high()));
	}

	if(initial_ranges.size() ==0) return;

	std::sort(initial_ranges.begin(), initial_ranges.end());
	long cur_range_low = initial_ranges[0].first;
	long cur_range_high = initial_ranges[0].second;

	for(int i=1; i<(int) initial_ranges.size(); i++)
	{
		long cur_low = initial_ranges[i].first;
		if(cur_low == cur_range_high+1){
			cur_range_high = initial_ranges[i].second;
			continue;
		}

		ranges.push_back(
				pair<long,long>(cur_range_low, cur_range_high));
		cur_range_low = cur_low;
		cur_range_high = initial_ranges[i].second;
	}
	ranges.push_back(
				pair<long,long>(cur_range_low, cur_range_high));

}


/*
void switch_statement::tighten_case_labels()
{
	for(int i=0; i < (int)case_labels->size(); i++)
	{
		case_label* cur_label = (*case_labels)[i];
		if(cur_label->is_default_label()) continue;
		for(int j=0; j<i; j++){
			case_label* processed_label = (*case_labels)[j];
			if(processed_label->is_default_label()) continue;
			if(cur_label->tightened_low >= processed_label->tightened_low
				&& cur_label->tightened_low <= processed_label->tightened_high)
			{
				cur_label->tightened_low = processed_label->tightened_high+1;
			}

			if(cur_label->tightened_high >= processed_label->tightened_low
				&& cur_label->tightened_high <= processed_label->tightened_high)
			{
				cur_label->tightened_high = processed_label->tightened_low-1;
			}
		}
	}
}
*/

}
