/*
 * SummaryStream.cpp
 *
 *  Created on: Nov 20, 2009
 *      Author: tdillig
 */

#include "SummaryStream.h"
#include "SummaryGraph.h"

#include "compass-serialization.h"

SummaryStream::SummaryStream(SummaryGraph* sg)
{

	write_summary_graph(data, sg);
}

Serializable *callback_summary_stream()
{
	return new SummaryStream();
}

stringstream& SummaryStream::get_data()
{
	return data;
}



SummaryStream::~SummaryStream()
{

}
