#ifndef _CIRCLE_SUMMARY_UNIT_H_
#define _CIRCLE_SUMMARY_UNIT_H_

#include "SummaryUnit.h"

class CircleSummaryUnit {

	vector<SummaryUnit*> su;
	vector<int> inst_size_list;

public:
	CircleSummaryUnit() {}

	~CircleSummaryUnit() {}

	CircleSummaryUnit& operator= (const CircleSummaryUnit& other);
	void add_summary_unit(SummaryUnit* sum);

	bool operator==(const CircleSummaryUnit& other) const;

	string to_string();
};

#endif
