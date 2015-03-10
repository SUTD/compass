/*
 * Compass.cpp
 *
 *  Created on: Dec 9, 2009
 *      Author: tdillig
 */

#include "Compass.h"
#include "Manager.h"
#include "DataManager.h"
#include "Callgraph.h"


#include "Error.h"
#include "util.h"

#include <vector>
#include <string>
#include <map>

#include "GlobalAnalysisState.h"
#include "MemoryAnalysis.h"
#include "AnalysisResult.h"


using namespace std;

Compass * c = NULL;

void start_fn(const Identifier & id)
{
	c->started(id);
}

void end_fn(const Identifier & id,  AnalysisResult * ar)
{
	c->finished(id, ar);
}



Compass::Compass(DataManager* dm, Callgraph* cg, int num_threads, int timeout,
		const Identifier & entry_point,
		set<Identifier>* excluded_functions):entry_point(entry_point),
		excluded_functions(excluded_functions)
{
	c = this;
	this->dm = dm;
	this->cg = cg;
	this->num_threads = num_threads;
	this->timeout = timeout;
	this->m = new Manager();

}

void Compass::run()
{
	GlobalAnalysisState::set_analysis_options(cg->get_checks_fn(), *dm);
	MemoryAnalysis ma;

	m->start(cg, false, &ma, dm, num_threads, 5, entry_point, excluded_functions,
			&start_fn, &end_fn);
	delete m;
	m = new Manager();

	emit finished();
}

void Compass::cancel()
{
	if(m == NULL) return;
	m->cancel();
}

void Compass::started(const Identifier & id)
{
	emit sum_started(QString::fromStdString(id.to_string()));
}
void Compass::finished(const Identifier & id, AnalysisResult * ar)
{
	errors.insert(ar->errors.begin(), ar->errors.end());
	emit sum_finished(QString::fromStdString(id.to_string()), ar);
}

set<Error*> & Compass::get_errors()
{
	return errors;
}




Compass::~Compass()
{

}
