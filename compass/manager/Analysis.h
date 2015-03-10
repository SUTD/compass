/*
 * Analysis.h
 *
 *  Created on: Aug 15, 2008
 *      Author: tdillig
 */

#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include <assert.h>
#include "serializable_types.h"
#include <set>
#include "call_id.h"
#include <string>
#include <map>
#include <set>
using namespace std;


class Analysis;
class Summary;
class SummaryFetcher;
class Report;
class SummaryGraph;

/**
 * Every client analysis needs to modify the following in this file:
 * 1) Add their analysis_kind id to the analysis_kind enum type.
 * 2) Add a string identifier to analyses array.
 * ***Important: We rely on the fact that the index of the string id in the
 * array has the same value as the analysis id in the enum.
 * 3) Construct the analysis of the right type in get_analysis function
 * at the end of this file.
 */

enum analysis_kind {
	GENERIC_ANALYSIS, // Just for testing for now
	COMPASS_ANALYSIS
};

/*
 * Command line name to invoke your analysis
 */
static const char* analyses[] =
{
		"generic"
		"compass"
};


enum analysis_order {
	TOP_DOWN,
	BOTTOM_UP
};



namespace sail {
	class Function;
	class SummaryUnit;
}

using namespace std;

class AnalysisResult;


class Analysis {

public:
	Analysis();
	virtual SummaryGraph* do_analysis(sail::SummaryUnit* f, bool report_errors,
			AnalysisResult *ar, SummaryFetcher* sr, int cg_id,
			map<il::type*, set<call_id> >* function_addresses, bool track_rtti);

	// Is this analysis top-down or bottom-up?
	virtual analysis_order get_order();

	virtual analysis_kind get_analysis_kind();

	virtual sum_data_type get_summary_type();

	virtual void register_summary_callbacks();

	bool has_summary();

	string get_summary_extension();

	/*
	 * Summaries of other analyses that are used by this analysis.
	 * The SummaryRetirever class will prefetch any dependent
	 * summaries associated with the currently analyzed function.
	 * For example, if analysis "Foo" uses analysis "PointerAnalysis",
	 * SummaryRetriever will prefetch the summary of PointerAnalysis
	 * for the currently analyzed function.
	 */
	virtual set<sum_data_type> & get_depedent_summary_types();

	virtual ~Analysis();
protected:
	set<sum_data_type> depedent_sum_types;
private:



	map<string, int> count;
};

Analysis* get_analysis(analysis_kind id);


#endif /* ANALYSIS_H_ */
