/*
 * SummaryViewer.h
 *
 *  Created on: Mar 11, 2010
 *      Author: tdillig
 */

#ifndef SUMMARYVIEWER_H_
#define SUMMARYVIEWER_H_

#include <qapplication.h>
#include <qimage.h>
#include <QtGui>
#include <string>
#include <map>
#include "Callgraph.h"

class SummaryGraph;
class SvgViewer;
class CGNode;
class ErrorTrace;

using namespace std;

class SummaryViewer : public QWidget
{
	Q_OBJECT
private:
	QLabel* su_label;
	QTabWidget* tabs;
	SvgViewer* drawer;
	QLineEdit* drawing_filter;
	QPushButton* redraw;
	SummaryGraph* sg;
	QTextEdit* return_cond;
	QTextEdit* loop_counters;


	QStandardItemModel* error_model;
	QTreeView *error_view;
	QTextEdit* cur_error_trace;

	map<QStandardItem*, ErrorTrace*> et;



public:
	SummaryViewer(QWidget* parent);
	void display(SummaryGraph* sg);
	void clear();
	virtual ~SummaryViewer();

public slots:
	void redraw_requested();
	void error_selected(const QModelIndex& mi);
};

#endif /* SUMMARYVIEWER_H_ */
