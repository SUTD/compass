/*
 * Regressions.cpp
 *
 *  Created on: Dec 11, 2009
 *      Author: tdillig
 */

#include "Regressions.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#include <iostream>
#include <assert.h>
#include <fstream>
#include "util.h"
#include "compass_ui.h"

#include "Manager.h"
#include "DataManager.h"
#include "Callgraph.h"
#include "SummaryStream.h"
#include "SummaryGraph.h"
#include "sail/SuperBlock.h"
#include "sail/SummaryUnit.h"

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

#include "RegressionRunner.h"
#include "fileops.h"



#define EXPECTED_FILE "errors.txt"



Regressions::Regressions(string folder, compass_ui* ui,
		QWidget* parent):QWidget(parent)
{
	this->ui = ui;
	this->folder = folder;
	rr = NULL;
	progress = new QProgressBar(this);
	regress_time = 0;


	num = new QLabel();
	status= new QLabel();
	time = new QLabel();
	num_passed = new QLabel();
	num_failed = new QLabel();

	reload_regressions();

	run = new QPushButton(this);
	run->setText("Run regressions");

	cancel = new QPushButton(this);
	cancel->setText("Cancel");
	cancel->setEnabled(false);


	connect(run, SIGNAL(clicked()),
	            this, SLOT(run_regressions()));
	connect(cancel, SIGNAL(clicked()),
	            this, SLOT(cancel_regressions()));

	QHBoxLayout* ll = new QHBoxLayout;
	{
		QLabel* l = new QLabel;
		l->setText("Total Regressions: ");
		ll->addWidget(l);
		ll->addWidget(num);

		l = new QLabel;
		l->setText("Passed: ");
		ll->addWidget(l);
		ll->addWidget(num_passed);

		l = new QLabel;
		l->setText("Failed: ");
		ll->addWidget(l);
		ll->addWidget(num_failed);


		ll->addWidget(status);
		l = new QLabel;
		l->setText("Total time: ");
		ll->addWidget(l);
		ll->addWidget(time);
		ll->addWidget(run);
		ll->addWidget(cancel);
	}

	model = new QStandardItemModel(this);
	view = new QTreeView(this);
	//view->setMaximumWidth(250);
	model->setColumnCount(4);
	QStringList l;
	l.push_back("Regression");
	l.push_back("Time");
	l.push_back("Status");
	l.push_back("Lines");
	model->setHorizontalHeaderLabels(l);
	view->setModel(model);


	QVBoxLayout* l1 = new QVBoxLayout;
	l1->addWidget(view);

	QHBoxLayout* ll2 = new QHBoxLayout;
	{
		show_only_errors = new QCheckBox("Show only failed regressions",this);
		connect(show_only_errors, SIGNAL(stateChanged(int)),
				               this, SLOT(box_clicked(int)));
		ll2->addWidget(show_only_errors);
		QLabel* l =new QLabel;
		l->setText("      Filter: ");
		ll2->addWidget(l);
		filter = new QLineEdit(this);

		connect(filter, SIGNAL(textEdited(const QString &)),
			this, SLOT(filter_changed(const QString &)));


		ll2->addWidget(filter);
		ll2->addStretch();

		num_cpus = new QSpinBox(this);
		num_cpus->setMinimum(1);
		int cpus_installed = sysconf(_SC_NPROCESSORS_CONF);
		num_cpus->setMaximum(cpus_installed);
		num_cpus->setValue(cpus_installed);
		ll2->addWidget(new QLabel("Num CPUs"));
		ll2->addWidget(num_cpus);
		ll2->setStretch(1, 0);
		ll2->setStretch(2, 0);


	}


	l1->addLayout(ll2);

	l1->addLayout(ll);



	l1->addWidget(progress);
	setLayout(l1);
	connect(view, SIGNAL(doubleClicked(QModelIndex)),
		               this, SLOT(regression_selected(QModelIndex)));

	update_view();

}

void Regressions::box_clicked(int ignore)
{
	bool show_errors = show_only_errors->isChecked();
	int j=0;
	map<string, set<Error> >::iterator it = saved_tests.begin();
	for(; it!= saved_tests.end(); it++, j++)
	{
		const string & k = it->first;
		if(matches_filter(k, filter->text().toStdString()))
		if(!show_errors)
		{
			view->setRowHidden(j, view->rootIndex(), false);
			continue;
		}
		view->setRowHidden(j, view->rootIndex(), true);

		bool passed = true;
		if(results.count(k) > 0) passed = results[k].first;
		if(!passed)
			view->setRowHidden(j, view->rootIndex(), false);


	}

}

bool Regressions::matches_filter(string  cur, string filter)
{
	if(filter.size() == 0) return true;
	transform(cur.begin(), cur.end(), cur.begin(), ptr_fun(::tolower));
	transform(filter.begin(), filter.end(), filter.begin(), ptr_fun(::tolower));
	return cur.find(filter)!=string::npos;
}



void Regressions::filter_changed(const QString& s)
{
	int j=0;
	map<string, set<Error> >::iterator it = saved_tests.begin();
	for(; it!= saved_tests.end(); it++, j++)
	{
		const string & k = it->first;
		bool passed = true;
		if(results.count(k) > 0) passed = results[k].first;
		if(!matches_filter(k, s.toStdString())){
			view->setRowHidden(j, view->rootIndex(), true);
		}
		else{
			if(!passed || !show_only_errors->isChecked())
				view->setRowHidden(j, view->rootIndex(), false);
		}
	}
}

void Regressions::collect_regressions(set<string> & reg_paths, string prefix)
{
	 DIR *dp =  opendir(prefix.c_str());
	 if(dp  == NULL) return;
	 struct dirent *dirp;
	 while ((dirp = readdir(dp)) != NULL) {
	     	string name = string(dirp->d_name);
	     	if(name == "." || name == "..")
	     		continue;
	     	string new_prefix = prefix + "/" + name;
	     	DIR *cur_dp = opendir(new_prefix.c_str());
	     	if(cur_dp  != NULL)
	     	{
	     		closedir(cur_dp);
	     		collect_regressions(reg_paths, new_prefix);
	     		continue;
	     	}
	     	if(name.find(".c")!=string::npos ||
	     			name.find(".java")!=string::npos ||
	     			name.find("AndroidManifest.xml")!=string::npos)
	     	{
	     		reg_paths.insert(prefix);
	     	}


	 }
	 closedir(dp);
}



void Regressions::run_regressions()
{
	num_pass = 0;
	num_fail = 0;
	regress_time = 0;
	time->setText("0");
	num_passed->setText("0");
	num_failed->setText("0");
	time->setText("0");

	run->setEnabled(false);
	system("rm -r /tmp/regress_*");

	delete rr;
	progress->setMaximum(model->rowCount());
	progress->setValue(0);

	rr = new RegressionRunner(saved_tests, this, this->num_cpus->value(),
			filter->text().toStdString());
	cancel->setEnabled(true);
	for(int i=0; i < model->rowCount(); i++)
	{
		model->setItem(i, 1, new QStandardItem(""));
		model->setItem(i, 2, new QStandardItem(""));

	}




	QObject::connect(rr,SIGNAL(finished_run()),this, SLOT(reg_finished()));






	rr->start();

}

void Regressions::cancel_regressions()
{
	rr->cancel();
}


void Regressions::regression_selected(QModelIndex i)
{
	int row = i.row();
	QStandardItem* it = model->item(row, 0);
	QString tt = it->text();
	string selected = tt.toStdString();
	string orig = selected;
	selected = folder + "/" + selected;

	QMessageBox msgBox;
	string text = "Overwrite current with \"" + orig + "\"?";
	msgBox.setText(QString::fromStdString(text));
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	int ret = msgBox.exec();
	if(ret != QMessageBox::Ok) return;

	ui->replace_project(selected, orig);


}

void Regressions::regression_finished(QString  id, double time, bool passed,
		bool crashed, int num_lines)
{
	if(crashed) passed = false;


	results[id.toStdString()] = make_pair(passed, time);
	string k = id.toStdString();
	string display = k.substr(folder.size() + 1);

	if(passed){
		num_pass++;
		string s = int_to_string(num_pass);
		num_passed->setText(QString::fromStdString(s));
	}
	else{
		num_fail++;
		string s = int_to_string(num_fail);
		num_failed->setText(QString::fromStdString(s));
	}

	regress_time+=time;
	string s = double_to_string(regress_time);
	this->time->setText(QString::fromStdString(s));

	progress->setValue(num_pass + num_fail);

	QList<QStandardItem *> list = model->findItems(QString::fromStdString(display));
	if(list.size() != 1) return;

	QStandardItem* item = *list.begin();
	int row = item->row();
	string t = double_to_string(time);
	QStandardItem* tt = new QStandardItem(QString::fromStdString(t));
	tt->setEditable(false);
	model->setItem(row, 1, tt);

	QStandardItem* p;
	if(passed) p = new QStandardItem(
			QIcon("/usr/share/icons/oxygen/32x32/actions/dialog-ok-apply.png"), "");
	else
	{
		/*
		 * Never hide errors
		 */
		view->setRowHidden(row, view->rootIndex(), false);

		if(!crashed) p = new QStandardItem(QIcon("/usr/share/icons/oxygen/32x32/actions/dialog-close.png"), "");
		else p = new QStandardItem(QIcon
				("/usr/share/icons/oxygen/16x16/actions/tools-report-bug"), "");
	}

	p->setEditable(false);
	model->setItem(row, 2, p);
	QStandardItem* ll = new QStandardItem(
				QString::fromStdString(int_to_string(num_lines)));
	ll->setEditable(false);
	model->setItem(row, 3, ll);


	//cout << "####\t" << num_lines << "\t" << t << endl;
}
void Regressions::reg_finished()
{

	run->setEnabled(true);
	cancel->setEnabled(false);
}

void Regressions::collect_regressions(set<string> & reg_paths)
{
	 DIR *dp;

	 if((dp  = opendir(folder.c_str())) == NULL) {
		 cout << "Error(" << errno << ") opening directory regressions "<< endl;
		 exit(1);
	 }

	 collect_regressions(reg_paths, folder);


}

void Regressions::build_expected_output(map<string, set<Error> > & result,
		set<string> &paths)
{
	set<string>::iterator it = paths.begin();
	for(; it!= paths.end(); it++)
	{
		string cur_path = *it;
		 DIR *dp = opendir(cur_path.c_str());
		 assert(dp != NULL);

		 set<Error>& errors = result[cur_path];
		 struct dirent *dirp;
		 while ((dirp = readdir(dp)) != NULL) {
			 string name = string(dirp->d_name);
			 if(name == string(EXPECTED_FILE))
			 {
				 ifstream in((cur_path+"/"+name).c_str());
				 while(!in.eof())
				 {
					 string cur_line;
					 std::getline(in, cur_line);
					 int line_end = cur_line.find(' ');
					 if(line_end == string::npos) break;
					 string line_num_str = cur_line.substr(0, line_end);
					 int line_num = string_to_int(line_num_str);
					 string error_str_1 = cur_line.substr(line_end+1);
					 int file_end = error_str_1.find(' ');



					 string error_str;
					 string filename;
					 if(file_end == string::npos)
					 {

						 error_str = error_str_1;
						 filename = "test.c";
					 }
					 else
					 {

						 error_str = error_str_1.substr(0, file_end);
						 filename = strip(error_str_1.substr(file_end+1));
						 if(filename == "")
						 {
							 filename = "test.c";
						 }

					 }
					 int error = string_to_int(error_str);
					 Error e(line_num, filename, (error_code_type) error, "",
							 Identifier());
					 errors.insert(e);

				 }
				 in.close();

			 }
		 }

		// cout << "EXPECTED ERRORS FOR PATH: " << cur_path << errors.size() << endl;

		 closedir(dp);
	}
}


void Regressions::reload_regressions()
{

	saved_tests.clear();
	set<string> paths;
	collect_regressions(paths);
	build_expected_output(saved_tests, paths);
	progress->setMaximum(paths.size());
	progress->setValue(0);
	string num_s = int_to_string(paths.size());
	num->setText(QString::fromStdString(num_s));
}

void Regressions::update_view()
{
	model->removeRows(0, model->rowCount());

	map<string, set<Error> >::iterator it = saved_tests.begin();
	int i=0;
	for(; it!= saved_tests.end(); it++, i++) {
		string k = it->first;


		string display = k.substr(folder.size() + 1);
		QStandardItem* q = new QStandardItem(QString::fromStdString(display));
		q->setEditable(false);



		model->appendRow( q);
		if(results.count(k) == 0){
			continue;
		}


		double time = results[k].second;
		string t = double_to_string(time);
		QStandardItem* tt = new QStandardItem(QString::fromStdString(t));
		tt->setEditable(false);
		model->setItem(i, 1, tt);

		bool passed = results[k].first;
		QStandardItem* p;
		if(passed) p = new QStandardItem(
				QIcon("/usr/share/icons/oxygen/32x32/actions/dialog-ok-apply.png"), "");
		else p = new QStandardItem(QIcon
				("/usr/share/icons/oxygen/32x32/actions/dialog-close.png"), "");
		p->setEditable(false);
		model->setItem(i, 2, p);

	}
	view->resizeColumnToContents(0);
	string num_s = int_to_string(saved_tests.size());
	num->setText(QString::fromStdString(num_s));

}


bool Regressions::regression_exists(const string & name)
{

	string key = folder + "/" + name;
	bool res = saved_tests.count(key) > 0;
	return res;
}


void Regressions::add_regression(string  name, const string & folder,
		set<Error> & errors)
{

	if(name[0] == '/') name = name.substr(1);
	if(name[name.size()-1] == '/') name = name.substr(0, name.size()-1);
	string reg_folder = this->folder + "/" + name + "/";
	remove_directory(reg_folder);
	copy_directory(folder, reg_folder);
	ofstream out;
	string error_file = reg_folder + EXPECTED_FILE;
	out.open(error_file.c_str(), ofstream::trunc);
	assert(out.is_open());
	set<Error>::iterator it = errors.begin();
	for(; it != errors.end(); it++)
	{
		out << it->line;
		out << " ";
		out << it->error;
		out << " ";
		out << it->file;
		out << "\n";
	}
	out.close();
	string key = this->folder + "/" + name;
	saved_tests[key] = errors;
	results.erase(key);
	update_view();


}


Regressions::~Regressions()
{

}
