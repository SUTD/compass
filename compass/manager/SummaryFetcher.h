/*
 * SummaryFetcher.h
 *
 *  Created on: Nov 20, 2009
 *      Author: tdillig
 */

#ifndef SUMMARYFETCHER_H_
#define SUMMARYFETCHER_H_
#include <string>
#include <map>
#include "Callgraph.h"
using namespace std;

class SummaryGraph;
class SummaryStream;
class SummaryFetcher {
private:

	map<call_id, SummaryStream*>* summary_stream_map;
	map<call_id, SummaryGraph*> cache;
	set<call_id> init_ids;
	const map<call_id, Identifier>& orig_fun_to_stub;
public:
	SummaryFetcher(map<call_id, SummaryStream*>* summary_stream_map,
			set<call_id>& init_ids,
			const map<call_id, Identifier>& orig_fun_to_stub);
	SummaryFetcher(map<Identifier, SummaryGraph*> & mm,
				set<call_id>& init_ids,
				const map<call_id, Identifier>& orig_fun_to_stub);

	SummaryGraph* get_summary(call_id  id);
	void get_init_function_summaries(vector<SummaryGraph*>& init_summaries);
	virtual ~SummaryFetcher();
};

#endif /* SUMMARYFETCHER_H_ */
