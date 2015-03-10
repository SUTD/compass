/*
 * Regressions.h
 *
 *  Created on: Dec 11, 2009
 *      Author: tdillig
 */

#ifndef REGRESSIONS_H_
#define REGRESSIONS_H_


#include <qgl.h>
#include <qapplication.h>
#include <qimage.h>
#include <QtGui>
#include <string>
#include <map>
#include <vector>
#include <set>

#include "Error.h"
class RegressionRunner;


using namespace std;


struct regression
{
	string code;
	vector<Error*> expected_output;
};

class compass_ui;


class Regressions : public QWidget{
	Q_OBJECT

public:
	Regressions(string folder, compass_ui* ui, QWidget* parent = NULL);
	virtual ~Regressions();
	bool regression_exists(const string & name);
	void add_regression(string  name, const string & folder,
			set<Error> & errors);

	static bool matches_filter(string  cur, string filter);

private:
	void reload_regressions();
	void collect_regressions(set<string> & reg_paths, string prefix);
	void collect_regressions(set<string> & reg_paths);
	void build_expected_output(map<string, set<Error> > & result,
			set<string> &paths);

	void update_view();

	compass_ui* ui;
	double regress_time;

	QStandardItemModel* model;
	QTreeView *view;
	string folder;

	QLabel* num;
	QLabel* status;
	QLabel* time;
	QLabel* num_passed;
	QLabel* num_failed;
	RegressionRunner * rr;
	QPushButton* run;
	QPushButton* cancel;

	QProgressBar *progress;
	QCheckBox* show_only_errors;
	QSpinBox* num_cpus;

	QLineEdit* filter;

	int num_pass;
	int num_fail;

	map<string, set<Error> > saved_tests;
	map<string, pair<bool, double> > results;





public slots:
	void run_regressions();
	void cancel_regressions();
	void regression_finished(QString  id, double  count, bool  passed,
			bool  crashed, int num_lines);
	void reg_finished();
	void regression_selected(QModelIndex i);
	void filter_changed(const QString& s);


	void box_clicked(int i);



};

#endif /* REGRESSIONS_H_ */
