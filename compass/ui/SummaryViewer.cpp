/*
 * SummaryViewer.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: tdillig
 */

#include "SummaryViewer.h"
#include "SvgViewer.h"
#include "SummaryGraph.h"
#include "ErrorTrace.h"
#include "util.h"
#include "compass_assert.h"

#include <set>

SummaryViewer::SummaryViewer(QWidget* parent):QWidget(parent)
{
	su_label = new QLabel();

	QVBoxLayout* l1 = new QVBoxLayout;
	l1->addWidget(su_label);

	tabs = new QTabWidget();
	tabs->setTabPosition(QTabWidget::South);
	l1->addWidget(tabs);

	drawer = new SvgViewer();


	drawing_filter = new QLineEdit;
	redraw  = new QPushButton;
	redraw->setText("Apply Filter");

	QObject::connect(redraw,SIGNAL(clicked()),this, SLOT(redraw_requested()));
	QObject::connect(drawing_filter,SIGNAL(returnPressed()),this,
			SLOT(redraw_requested()));

	QHBoxLayout* l2 = new QHBoxLayout;
	l2->addWidget(drawing_filter);
	l2->addWidget(redraw);

	QVBoxLayout* draw_tab_layout = new QVBoxLayout;
	draw_tab_layout->addWidget(drawer);
	draw_tab_layout->addLayout(l2);
	QWidget* draw_widget = new QWidget;
	draw_widget->setLayout(draw_tab_layout);

	this->setLayout(l1);


	tabs->addTab(draw_widget, "Summary Effect");



	QSplitter* splitter = new QSplitter;
	splitter->setChildrenCollapsible(false);

	/*
	 * Return cond
	 */
	return_cond = new QTextEdit;
	return_cond->setReadOnly(true);

	QVBoxLayout* ret_layout = new QVBoxLayout;
	QLabel* ret_label = new QLabel;
	ret_label->setText("Return Condition:");

	ret_layout->addWidget(ret_label);
	ret_layout->addWidget(return_cond);

	QWidget* ret_widget = new QWidget;
	ret_widget->setLayout(ret_layout);
	splitter->addWidget(ret_widget);

	/*
	 * Loop counters
	 */

	loop_counters = new QTextEdit;
	loop_counters->setReadOnly(true);

	QVBoxLayout* lc_layout = new QVBoxLayout;
	QLabel* lc_label = new QLabel;
	lc_label->setText("Loop Counter Relations:");

	lc_layout->addWidget(lc_label);
	lc_layout->addWidget(loop_counters);

	QWidget* lc_widget = new QWidget;
	lc_widget->setLayout(lc_layout);
	splitter->addWidget(lc_widget);


	tabs->addTab(splitter, "Return Condition/Loop Counters");

	//-------------------------------------------------------------

	QSplitter* error_splitter = new QSplitter;
	error_splitter->setChildrenCollapsible(false);

	error_view = new QTreeView;
	error_model = new QStandardItemModel(this);
	error_view->setModel(error_model);

	error_splitter->addWidget(error_view);

	cur_error_trace = new QTextEdit;
	cur_error_trace->setReadOnly(true);
	error_splitter->addWidget(cur_error_trace);

	tabs->addTab(error_splitter, "Error Traces");
	QList<int> init_sizes;
	init_sizes.push_back(200);
	init_sizes.push_back(600);
	error_splitter->setSizes(init_sizes);


	connect(error_view, SIGNAL(clicked(const QModelIndex&)),
	               this, SLOT(error_selected(const QModelIndex&)));

	clear();

}


void SummaryViewer::redraw_requested()
{
	if(sg == NULL) return;
	string filter = drawing_filter->text().toStdString();
	drawer->draw(sg->to_dotty(filter));

}

void SummaryViewer::display(SummaryGraph* sg)
{
	clear();
	if(sg == NULL){

		su_label->setText("No summary available for this unit.");
		return;
	}
	this->sg = sg;

	string res = sg->get_call_id().to_string();
	if(res.size()>30) {

		int index = res.find('(');
		if(index != string::npos){

			res = res.substr(0, index);
		}
	}


	su_label->setText("Summary Unit: " +
			QString::fromStdString(res));
	drawer->draw(sg->to_dotty(""));

	Constraint ret_c = sg->get_return_cond();
	return_cond->setText(QString::fromStdString(ret_c.to_string()));

	QString loop_text;
	const set<Constraint> & loop_c = sg->get_loop_counter_relations();
	set<Constraint>::iterator it = loop_c.begin();
	for(; it != loop_c.end(); it++)
	{
		Constraint cur_c = *it;
		loop_text+= QString::fromStdString(cur_c.to_string());
		loop_text+= "\n\n";
	}
	if(loop_c.size() == 0)
		loop_text = "[none]";
	loop_counters->setText(loop_text);

	/*
	 * Sort traces by type
	 */
	const set<ErrorTrace*> & error_traces = sg->get_error_traces();
	set<ErrorTrace*>::iterator it2 = error_traces.begin();

	map<trace_type, set<ErrorTrace*> > errors;
	for(;it2 != error_traces.end(); it2++)
	{
		ErrorTrace* et = *it2;
		errors[et->get_trace_type()].insert(et);
	}

	map<trace_type, set<ErrorTrace*> >::iterator it3 = errors.begin();
	for(; it3 != errors.end(); it3++)
	{
		trace_type cur = it3->first;
		string cur_kind = ErrorTrace::trace_type_to_string(cur);
		QList<QStandardItem*> entry;



		QStandardItem *id2 = new QStandardItem(QString::fromStdString(cur_kind));
		id2->setEditable(false);
		entry.push_back(id2);


		error_model->appendRow(entry);
		set<ErrorTrace*> & traces = it3->second;
		set<ErrorTrace*>::iterator it4 = traces.begin();
		for(; it4 != traces.end(); it4++)
		{
			ErrorTrace* cur = *it4;
			QList<QStandardItem*> entry;
			QStandardItem *id3 = new QStandardItem(
					QString::fromStdString(cur->get_id()));
			id3->setEditable(false);
			et[id3] = cur;
			entry.push_back(id3);
			id2->appendRow(entry);

		}
	}

	error_view->expandAll();
	error_view->resizeColumnToContents(0);
}


void SummaryViewer::error_selected(const QModelIndex& mi)
{

	QStandardItem *error_id = error_model->itemFromIndex(mi);
	if(et.count(error_id) == 0)
	{
		cur_error_trace->setText("");
		return;
	}
	ErrorTrace* trace = et[error_id];
	cur_error_trace->setText(QString::fromStdString(trace->to_string()));
}

void SummaryViewer::clear()
{
	sg = NULL;
	su_label->setText("No Summary Unit selected.");
	drawing_filter->setText("");
	return_cond->setText("");
	loop_counters->setText("");

	error_model->clear();
	error_model->setColumnCount(1);
	QStringList l;
	l.push_back("Error Traces");
	error_model->setHorizontalHeaderLabels(l);

	cur_error_trace->setText("");
}


SummaryViewer::~SummaryViewer()
{

}
