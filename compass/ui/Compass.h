/*
 * Compass.h
 *
 *  Created on: Dec 9, 2009
 *      Author: tdillig
 */

#ifndef COMPASS_H_
#define COMPASS_H_
#include <string>
#include <set>
using namespace std;

#include <QtGui>
#include "Identifier.h"

class DataManager;
class Callgraph;
class Error;
class AnalysisResult;
class Manager;

class Compass: public QThread {
	Q_OBJECT
private:
	DataManager *dm;
	int num_threads;
	Callgraph* cg;
	int timeout;
	set<Error*> errors;
	Manager* m;
	const Identifier& entry_point;
	set<Identifier>* excluded_functions;

public:
	Compass(DataManager* dm, Callgraph* cg, int num_thread, int timeout,
			const Identifier & entry_point,
			set<Identifier>* excluded_functions);
	virtual ~Compass();
	void started(const Identifier & id);
	void finished(const Identifier & id, AnalysisResult * ar);
	void cancel();
	set<Error*> & get_errors();
protected:
    void run();
signals:
	void finished();
	void sum_started(QString id);
	void sum_finished(QString, AnalysisResult * ar);

};

#endif /* COMPASS_H_ */
