/*
 * RegressionRunner.h
 *
 *  Created on: Dec 12, 2009
 *      Author: tdillig
 */

#ifndef REGRESSIONRUNNER_H_
#define REGRESSIONRUNNER_H_


#include <QtGui>
#include <string>
#include <set>
using namespace std;

class DataManager;
class Callgraph;
class Error;
class AnalysisResult;
class Regressions;

class RegressionRunner: public QThread  {

	Q_OBJECT

	friend class RegressionItem;

public:
	RegressionRunner(map<string, set<Error> > &saved_tests, Regressions* r,
			int num_threads, string filter);
	virtual ~RegressionRunner();
	void cancel();
protected:
    void run();

    map<string, set<Error> >& saved_tests;
    Regressions* r;
    int num_threads;
    QMutex cancel_mutex;
    set<int> p_ids;
    bool canceled;
    string filter;




signals:
	void finished_run();
	void regression_finished(QString  id, double count, bool passed,
			bool crashed, int num_lines);
public slots:
void unit_finished(QString  id, double count, bool passed,
			bool crashed, int num_lines);


};

#endif /* REGRESSIONRUNNER_H_ */
