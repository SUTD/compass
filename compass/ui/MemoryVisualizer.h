/*
 * MemoryVisualizer.h
 *
 *  Created on: Dec 10, 2009
 *      Author: tdillig
 */

#ifndef MEMORYVISUALIZER_H_
#define MEMORYVISUALIZER_H_

#include <qgl.h>
#include <qapplication.h>
#include <qimage.h>
#include <QtGui>
#include <string>
#include <map>
#include "Callgraph.h"
#include "MemoryAnalysis.h"

namespace sail
{
	class SummaryUnit;
}

class SummaryGraph;
class SvgViewer;
class CGNode;
class DataManager;

using namespace std;

class MemoryVisualizer : public QWidget
{
	Q_OBJECT

public:
	MemoryVisualizer(QWidget* parent);

	void load(
			sail::SummaryUnit*, map<Identifier, SummaryGraph*> * sums,
			CGNode* node,
			map<il::type*, set<call_id> >* function_addresses,
			Callgraph* cg);
	void clear();
	virtual ~MemoryVisualizer();
private:

	void select(QStandardItem* item);
	int read_restart(int fd, void *buf, size_t count);
	void crash_handler_main(int p_id, int* status_pipe);


	QLabel* fun;
	QStandardItemModel* statements;
	QTreeView *stmt_tree;
	SvgViewer* drawer;
	QPushButton* run;

	QTextEdit* output;
	QCheckBox* display_all;
	QCheckBox* same_process;

	map<Identifier, SummaryGraph*> * sums;
	vector<dotty_info> dotties;
	map<il::type*, set<call_id> >* function_addresses;

	sail::SummaryUnit* su;
	QStandardItem* selected;
	CGNode* node;
	Callgraph* cg;

	map<QStandardItem*, dotty_info> to_display;

	public slots:
		void run_analysis();

		void run_memory_analysis(vector<dotty_info> * _dotties,
				AnalysisResult * _ar, double * _time);
		void stmt_selected(QModelIndex index);
		void box_changed();



};

#endif /* MEMORYVISUALIZER_H_ */
