/*
 * SummaryStream.h
 *
 *  Created on: Nov 20, 2009
 *      Author: tdillig
 */

#ifndef SUMMARYSTREAM_H_
#define SUMMARYSTREAM_H_

#include "Serializable.h"
#include <sstream>
using namespace std;

class SummaryGraph;


class SummaryStream: public Serializable {
	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		string temp = data.str();
		ar & temp;
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		string s;
		ar & s;
		data << s;


	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
	stringstream data;
public:
	SummaryStream(){};
	SummaryStream(SummaryGraph* sg);
	virtual ~SummaryStream();
	stringstream& get_data();
};

Serializable *callback_summary_stream();


#endif /* SUMMARYSTREAM_H_ */
