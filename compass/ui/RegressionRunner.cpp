/*
 * RegressionRunner.cpp
 *
 *  Created on: Dec 12, 2009
 *      Author: tdillig
 */

#include "RegressionRunner.h"
#include "CNode.h"
#include "util.h"
#include "compass_ui.h"

#include "RegressionItem.h"

#include "Manager.h"
#include "DataManager.h"
#include "Callgraph.h"
#include "SummaryStream.h"
#include "SummaryGraph.h"
#include "sail/SuperBlock.h"
#include "sail/SummaryUnit.h"
#include "Regressions.h"

#include "Error.h"
#include "util.h"
#include <vector>
#include "GlobalAnalysisState.h"
#include "MemoryAnalysis.h"
#include "compass-serialization.h"
#include "Compass.h"
#include "GlobalAnalysisState.h"
#include "MemoryAnalysis.h"
#include "AnalysisResult.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <QSemaphore>


#define CANCEL_EXIT_STATUS 9

RegressionRunner::RegressionRunner(map<string, set<Error> > &saved_tests,
		Regressions* r, int num_threads, string filter):
saved_tests(saved_tests), r(r)
{
	this->filter = filter;
	canceled = false;
	this->num_threads = num_threads;

}


void RegressionRunner::cancel()
{
	cancel_mutex.lock();
	canceled = true;
	set<int>::iterator it = p_ids.begin();

	for(; it != p_ids.end(); it++)
	{
		int p_id = *it;
		kill(p_id, CANCEL_EXIT_STATUS);
	}
	cancel_mutex.unlock();
}




void RegressionRunner::run()
{
	map<string, set<Error> >::iterator it = saved_tests.begin();
	int i = 0;


	RegressionItem *completed[saved_tests.size()];
	memset(completed, 0, sizeof(RegressionItem*)*saved_tests.size());

	QSemaphore qs(num_threads);
	for(; it != saved_tests.end(); it++, i++)
	{
		cancel_mutex.lock();

		if(canceled){
			cancel_mutex.unlock();
			break;
		}
		if(filter.size() > 0)
		{
			if(!Regressions::matches_filter(it->first, filter))
			{
				cancel_mutex.unlock();
				continue;
			}
		}

		cancel_mutex.unlock();
		qs.acquire();

		string folder = it->first;

		set<Error>& expected_output = it->second;
		RegressionItem* ri = new RegressionItem(folder, expected_output, this,
				r, qs, i);
		completed[i] = ri;

		ri->start();

	}



	qs.acquire(num_threads);


	for(unsigned int i=0; i < saved_tests.size(); i++)
		delete completed[i];

	emit this->finished_run();

}

void RegressionRunner::unit_finished(QString  id, double time, bool success,
			bool crashed, int num_lines)
{
	emit regression_finished(id, time, success, crashed, num_lines);
}





RegressionRunner::~RegressionRunner()
{

}
