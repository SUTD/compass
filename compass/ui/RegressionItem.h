/*
 * RegressionItem.h
 *
 *  Created on: Jan 4, 2010
 *      Author: tdillig
 */

#ifndef REGRESSIONITEM_H_
#define REGRESSIONITEM_H_

#include <QtGui>
#include <string>
#include <set>
#include <Error.h>
#include <QSemaphore>

using namespace std;

class DataManager;
class Callgraph;
class Error;
class AnalysisResult;
class RegressionRunner;
class Regressions;

class RegressionItem : public QThread
{
	Q_OBJECT

private:
	string folder;
	set<Error> & expected_output;
	QSemaphore & s;
	int c;
	RegressionRunner* rr;
public:
	RegressionItem(string folder, set<Error> & expected_output,
			RegressionRunner* rr, Regressions* r,
			QSemaphore & s, int c);
	~RegressionItem() {}
protected:
	 void run();
	 bool run_regression_in_given_folder(string folder,
	    			set<Error> & expected_output, double & time, bool & crashed,
	    			int & num_lines);
signals:
	 void unit_finished(QString  id, double count, bool passed,
	 			bool crashed, int num_lines);
};


#endif /* REGRESSIONITEM_H_ */
