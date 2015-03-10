/*
 * SummaryFetcher.cpp
 *
 *  Created on: Nov 20, 2009
 *      Author: tdillig
 */

#include "SummaryFetcher.h"
#include "Manager.h"
#include "SummaryStream.h"

#include "compass-serialization.h"




SummaryFetcher::SummaryFetcher(map<call_id, SummaryStream*>* summary_stream_map,
		set<call_id>& init_ids, const map<call_id, Identifier>& orig_fun_to_stub)
		: orig_fun_to_stub(orig_fun_to_stub)
{
	this->summary_stream_map = summary_stream_map;
	this->init_ids = init_ids;
}

SummaryFetcher::SummaryFetcher(map<Identifier, SummaryGraph*> & mm,
			set<call_id>& init_ids,
			const map<call_id, Identifier>& orig_fun_to_stub)
			: orig_fun_to_stub(orig_fun_to_stub)
{
	this->summary_stream_map = NULL;
	this->init_ids = init_ids;
	map<Identifier, SummaryGraph*>::iterator it = mm.begin();
	for(; it != mm.end(); it++)
	{
		call_id cid(it->first);
		cache[cid] = it->second;
	}
}

void SummaryFetcher::get_init_function_summaries(vector<SummaryGraph*>&
		init_summaries)
{
	set<call_id>::iterator it = init_ids.begin();
	for(; it!= init_ids.end(); it++)
	{
		call_id cur = *it;
		SummaryGraph* sg = get_summary(cur);
		init_summaries.push_back(sg);
	}

}

SummaryGraph* SummaryFetcher::get_summary(call_id  id)
{
	if(orig_fun_to_stub.count(id) > 0) {
		const Identifier& long_id = orig_fun_to_stub.find(id)->second;
		id = call_id(long_id);
	}

	if(cache.count(id) > 0) {
		return cache[id];
	}

	if(summary_stream_map != NULL)
	{
		SummaryStream* ss = (*summary_stream_map)[id];

		if(ss == NULL){
			cache[id] = NULL;
			return NULL;
		}
		SummaryGraph* new_sg = load_summary_graph(ss->get_data());
		cache[id] = new_sg;
		return new_sg;

	}

	/*
	 * Only triggered from MemoryVisualizer
	 */
	return NULL;
}

SummaryFetcher::~SummaryFetcher()
{

}
