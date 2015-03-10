/*
 * SummaryRetriever.cpp
 *
 *  Created on: Aug 15, 2008
 *      Author: tdillig
 */

#include "SummaryRetriever.h"
#include "Summary.h"
#include "Analysis.h"
#include "sail/Function.h"
#include "Callgraph.h"
#include "SummaryStream.h"

SummaryRetriever::SummaryRetriever(DataManager *dm, CGNode *cur,
		Analysis *a)
{
	this->dm = dm;
	bool bottom_up = a->get_order() == BOTTOM_UP;
	sum_data_type cur_sum_type = a->get_summary_type();
	set<sum_data_type> & dependent_sum_types = a->get_depedent_summary_types();
	set<sum_data_type>::iterator it = dependent_sum_types.begin();
	for(; it!= dependent_sum_types.end(); it++)
	{
		dm->prefetch_data(cur->get_serial_id(), *it);
	}
	if(cur_sum_type == NO_DATA)
		return;

	map<CGNode*, cg_edge_id> & dependencies = bottom_up? cur->callees: cur->callers;
	map<CGNode*, cg_edge_id>::iterator it2 = dependencies.begin();
	for(; it2 != dependencies.end(); it2++)
	{
		CGNode* cgn = it2->first;
		dm->prefetch_data(cgn->get_serial_id(), cur_sum_type);
	}
	dm->prefetch_data(cur->get_serial_id(), cur_sum_type);

}

SummaryStream* SummaryRetriever::get_summary(string f_id, sum_data_type type)
{
	SummaryStream* s = (SummaryStream*)dm->get_data(f_id, type);
	requested.insert(pair<string, sum_data_type>(f_id, type));
	return s;
}


SummaryRetriever::~SummaryRetriever()
{
	set<pair<string, sum_data_type> >::iterator it = requested.begin();
	for(; it != requested.end(); it++)
	{
		dm->mark_unused(it->first, it->second);
	}
}
