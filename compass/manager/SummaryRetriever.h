/*
 * SummaryRetriever.h
 *
 *  Created on: Aug 15, 2008
 *      Author: tdillig
 */

#ifndef SUMMARYRETRIEVER_H_
#define SUMMARYRETRIEVER_H_

#include "Analysis.h"
#include "DataManager.h"
#include <set>
using namespace std;

class Summary;
namespace sail {
	class Function;
}

class CGNode;
class SummaryStream;

using namespace std;
using namespace sail;

class SummaryRetriever {
private:
	set<pair<string, sum_data_type> > requested;
	DataManager *dm;
public:
	SummaryRetriever(DataManager *dm, CGNode *cur,
			Analysis *a);
	SummaryStream* get_summary(string f_id, sum_data_type type);
	virtual ~SummaryRetriever();
};

#endif /* SUMMARYRETRIEVER_H_ */
