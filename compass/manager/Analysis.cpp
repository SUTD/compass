/*
 * Analysis.cpp
 *
 *  Created on: Aug 15, 2008
 *      Author: tdillig
 */

#include "Analysis.h"
#include "Summary.h"
#include "GenericSummary.h"
#include "sail/Function.h"
#include "SummaryFetcher.h"

#include "Serializable.h"
#include "util.h"
#include "MemoryAnalysis.h"


using namespace sail;

Analysis::Analysis()
{

}

SummaryGraph* Analysis::do_analysis(SummaryUnit* f,
		bool report_errors, AnalysisResult *ar, SummaryFetcher* sr, int cg_id,
		map<il::type*, set<call_id> >* function_addresses, bool track_rtti)
{
	return NULL;

}

/*
 * Top-down analyses must override this method.
 */
analysis_order Analysis::get_order()
{
	return BOTTOM_UP;
}

analysis_kind Analysis::get_analysis_kind()
{
	return GENERIC_ANALYSIS;
}

sum_data_type Analysis::get_summary_type()
{
	return COMPASS_SUMMARY;
}


set<sum_data_type> & Analysis::get_depedent_summary_types()
{
	return depedent_sum_types;
}

Serializable *callback_generic()
{
	return new GenericSummary();
}

void Analysis::register_summary_callbacks()
{

}

bool Analysis::has_summary()
{
	return get_summary_type() != NO_DATA;
}

string Analysis::get_summary_extension()
{
	assert(has_summary());
	assert((int)sizeof(file_extensions)/sizeof(char*) > (int)get_summary_type());
	return file_extensions[get_summary_type()];
}

Analysis* get_analysis(analysis_kind id)
{
	switch(id)
	{
		case GENERIC_ANALYSIS:
			return new Analysis();
		case COMPASS_ANALYSIS:
			return new MemoryAnalysis();
		default:
			assert(false);
	}
}






Analysis::~Analysis() {
	// TODO Auto-generated destructor stub
}
