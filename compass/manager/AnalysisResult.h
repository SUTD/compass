/*
 * AnalysisResult.h
 *
 *  Created on: Nov 23, 2009
 *      Author: tdillig
 */

#ifndef ANALYSISRESULT_H_
#define ANALYSISRESULT_H_

#include <vector>
using namespace std;
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

#include "Error.h"

class AnalysisResult {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & errors;
		ar & time;
		ar & termination_value;
		ar & constraint_stats;
	}
public:
	vector<Error*> errors;
	double time;

	/*
	 * The termination_value of the process that ran the analysis.
	 * If this value is not 0, no other field is set.
	 */
	int termination_value;

	string constraint_stats;
public:
	AnalysisResult();
	virtual ~AnalysisResult();
};

#endif /* ANALYSISRESULT_H_ */
