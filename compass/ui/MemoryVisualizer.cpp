/*
 * MemoryVisualizer.cpp
 *
 *  Created on: Dec 10, 2009
 *      Author: tdillig
 */

#include "MemoryVisualizer.h"


#include "Manager.h"
#include "DataManager.h"
#include "Callgraph.h"
#include "SummaryStream.h"
#include "SummaryGraph.h"

#include "Error.h"
#include "util.h"
#include <vector>

#include "GlobalAnalysisState.h"
#include "MemoryAnalysis.h"
#include "compass-serialization.h"
#include "SvgViewer.h"
#include "SummaryFetcher.h"
#include "AnalysisResult.h"

#include "compass_ui.h"


#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include "compass-serialization.h"

#include <boost/thread.hpp>

#define TOP_OF_CALLG false

MemoryVisualizer::MemoryVisualizer(QWidget *parent):QWidget(parent)
{



	fun = new QLabel(this);
	statements = new QStandardItemModel(this);
	stmt_tree = new QTreeView(this);

	stmt_tree->setModel(statements);

	drawer = new SvgViewer(this);

	QVBoxLayout* l3 = new QVBoxLayout;
	l3->addWidget(stmt_tree);

	run = new QPushButton(this);
	run->setText("Run");
	run->setEnabled(false);

	display_all = new QCheckBox("Show full graph");
	same_process = new QCheckBox("Run in same process");
	QHBoxLayout* check_layout = new QHBoxLayout;
	check_layout->addWidget(display_all);
	check_layout->addWidget(same_process);
	l3->addLayout(check_layout);

	l3->addWidget(run);

	QSplitter* v_splitter = new QSplitter;
	QWidget* control = new QWidget;
	control->setLayout(l3);
	v_splitter->addWidget(control);
	v_splitter->addWidget(drawer);
	v_splitter->setChildrenCollapsible(false);

	QList<int> v_init_sizes;
	v_init_sizes.push_back(300);
	v_init_sizes.push_back(600);
	v_splitter->setSizes(v_init_sizes);


	QVBoxLayout* l2 = new QVBoxLayout();
	l2->addWidget(fun);



	output = new QTextEdit(this);
	output->document()->setPlainText("");
	output->setTextInteractionFlags(Qt::TextBrowserInteraction);


	QSplitter* h_splitter = new QSplitter;
	h_splitter->setOrientation(Qt::Vertical);
	h_splitter->addWidget(v_splitter);
	h_splitter->addWidget(output);
	h_splitter->setChildrenCollapsible(false);

	l2->addWidget(h_splitter);

	QList<int> h_init_sizes;
	h_init_sizes.push_back(800);
	h_init_sizes.push_back(130);
	h_splitter->setSizes(h_init_sizes);


	setLayout(l2);

	connect(run, SIGNAL(clicked()),
	            this, SLOT(run_analysis()));



    connect(stmt_tree, SIGNAL(activated(QModelIndex)),
			   this, SLOT(stmt_selected(QModelIndex)));
    connect(stmt_tree, SIGNAL(clicked(QModelIndex)),
   			   this, SLOT(stmt_selected(QModelIndex)));

    connect(display_all, SIGNAL(stateChanged(int)),
   			   this, SLOT(box_changed()));




 	clear();


}

void MemoryVisualizer::box_changed()
{
	select(selected);
}


void MemoryVisualizer::stmt_selected(QModelIndex index)
{
	select(statements->itemFromIndex(index));
}

int MemoryVisualizer::read_restart(int fd, void *buf, size_t count)
{
	while(true)
	{
		int res = read(fd, buf, count);
		if(res>= 0) return res;
		assert(errno == 4);
	}

}

void MemoryVisualizer::crash_handler_main(int p_id, int* status_pipe)
{
	int child_status;

	waitpid(p_id, &child_status, WUNTRACED);


	if(child_status == 0) return;
	double time = -1;
	int s = write(status_pipe[1], &time, sizeof(double));

	int error = -1;
	s = write(status_pipe[1], &error, sizeof(int));
	assert(s != -1);

}


void MemoryVisualizer::run_memory_analysis(vector<dotty_info> * _dotties,
		AnalysisResult * _ar, double * _time)
{
	vector<dotty_info> & dotties = *_dotties;

	AnalysisResult & ar = *_ar;
	double & time = *_time;

	MemoryAnalysis ma;
	set<call_id> init;
	{
		const map<CGNode*, cg_edge_id> & callees = node->get_callees();
		map<CGNode*, cg_edge_id>::const_iterator it = callees.begin();
		for(; it!= callees.end(); it++)
		{
			CGNode* key = it->first;
			call_id ci(key->id);
			if(cg->get_init_functions().count(key) > 0) {
				init.insert(ci);
			}
		}
	}
	SummaryFetcher sf(*sums, init, cg->get_stubs());
	CGNode* fun_cg_node = cg->get_node(node->id.get_function_identifier());
	int fun_cg_id = fun_cg_node->cg_id;

	/*
	 * Simple in the same process
	 */
	if(same_process->isChecked())
	{
		int start = clock();
		ma.do_analysis(su, TOP_OF_CALLG,
				&sf, &ar, &dotties,  fun_cg_id, function_addresses,
				cg->track_rtti());
		int time = clock() - start;
		time = ((double)time)/((double)CLOCKS_PER_SEC);
		return;
	}

	/*
	 * Here, run the memory graph in a different process
	 */
	int p[2];
	int pipe_status = pipe(p);
	assert(pipe_status == 0);
	int p_id = fork();



	/*
	 * New process
	 */
	if(p_id == 0)
	{
		//make sure we die if parent dies
		prctl(PR_SET_PDEATHSIG, SIGHUP);



		AnalysisResult  ar;
		int start = clock();
		ma.do_analysis(su, TOP_OF_CALLG,
				&sf, &ar, &dotties,  fun_cg_id, function_addresses,
				cg->track_rtti());
		int time = clock() - start;
		double t = ((double)time)/((double)CLOCKS_PER_SEC);

		/*
		 *
		 */
		int s = write(p[1], &t, sizeof(double));
		assert(s != -1);
		int size = dotties.size();
		s = write(p[1], &size, sizeof(int));
		assert(s != -1);
		vector<dotty_info>::iterator it = dotties.begin();
		for(; it != dotties.end(); it++)
		{
			dotty_info & cur = *it;
			stringstream ss;
			write_dotty_info(ss, &cur);
			int cur_size = ss.str().size();
			s = write(p[1], &cur_size, sizeof(int));
			assert(s != -1);
			s = write(p[1], ss.str().c_str(), cur_size);
			assert(s != -1);
		}
		stringstream ss;
		write_analysis_results(ss, &ar);
		int ar_size = ss.str().size();
		s = write(p[1], &ar_size, sizeof(int));
		assert(s != -1);
		s = write(p[1], ss.str().c_str(), ar_size);
		assert(s != -1);
		exit(0);
	}

	boost::thread crash_handler = boost::thread(bind(
			&MemoryVisualizer::crash_handler_main, this, p_id, p));




	double t;
	read_restart(p[0], &t, sizeof(double));
	int num_dotties;
	read_restart(p[0], &num_dotties, sizeof(int));
	stringstream ss;
	AnalysisResult* new_ar;
	if(num_dotties == -1) goto end;



	for(int i=0; i<num_dotties; i++)
	{
		int cur_size = 0;
		if(cur_size == -1){
			t = -1;
			goto end;
		}
		read_restart(p[0], &cur_size, sizeof(int));
		char buf[cur_size];
		read_restart(p[0], &buf[0], cur_size);
		stringstream ss;
		for(int j=0; j <cur_size; j++)
		{
			ss << buf[j];
		}
		dotty_info* di = load_dotty_info(ss);
		dotties.push_back(*di);
		delete di;
	}
	int ar_size;
	read_restart(p[0], &ar_size, sizeof(int));
	if(ar_size == -1){
		t = -1;
		goto end;
	}
	char buf[ar_size];
	read_restart(p[0], &buf[0], ar_size);

	for(int j=0; j <ar_size; j++)
	{
		ss << buf[j];
	}
	new_ar = load_analysis_result(ss);
	ar = *new_ar;
	delete new_ar;


end:

	crash_handler.join();

	::close(p[0]);
	::close(p[1]);

	time = t;


}

void MemoryVisualizer::run_analysis()
{

	run->setEnabled(false);
	if(sums == NULL) return;
	AnalysisResult ar;
	double t = 0.0;
	boost::thread run = boost::thread(
			boost::bind(&MemoryVisualizer::run_memory_analysis, this,
			&dotties, &ar, &t));
	run.join();



	vector<dotty_info >::iterator it = dotties.begin();

	vector<QStandardItem*> parents;
	QStandardItem* last_item = statements->invisibleRootItem();
	parents.push_back(statements->invisibleRootItem());
	for(; it!= dotties.end(); it++)
	{
		const dotty_info & di = *it;
		//cout << "DI: " << di.level << "  " << di.key << endl;

		//Adding level
		if(di.level == (int)parents.size())
		{

			QStandardItem* qs =
					new QStandardItem(QString::fromStdString(di.key));
			qs->setEditable(false);
			last_item->appendRow(qs);
			parents.push_back(last_item);
			to_display[qs] = di;
			last_item = qs;
			continue;

		}//removing level
		else if(di.level == (int)parents.size()-2)
		{
			assert(parents.size() > 1);
			parents.pop_back();
		}


		QStandardItem* qs = new QStandardItem(QString::fromStdString(di.key));
		qs->setEditable(false);
		parents[parents.size()-1]->appendRow(qs);
		to_display[qs] = di;
		last_item = qs;
	}


	stmt_tree->expandToDepth(0);
	stmt_tree->resizeColumnToContents(0);


	QString res;
	if(t >=0) res = "Time: " + QString::fromStdString(double_to_string(t));
	else res = "***** Crashed ***** ";

	for(unsigned int i=0; i < ar.errors.size(); i++)
	{
		Error* e = ar.errors[i];
		res += QString::fromStdString(e->to_string()) + "\n";
	}
	output->document()->setPlainText(res);

}



void MemoryVisualizer::select(QStandardItem* item)
{

	selected = item;
	if(to_display.count(item) == 0){
		drawer->clear();
		return;
	}

	//cout << "dotty all: " << to_display[item].val2 << endl;
	if(display_all->isChecked())
		drawer->draw(to_display[item].val2);
	else drawer->draw(to_display[item].val1);


}

void MemoryVisualizer::load(sail::SummaryUnit* su,
		map<Identifier, SummaryGraph*> * sums, CGNode* node,
		map<il::type*, set<call_id> >* function_addresses,
		Callgraph* cg)
{
	clear();
	this->node = node;
	this->cg = cg;
	string res = call_id(su->get_identifier()).to_string();
	if(res.size()>30) {

		int index = res.find('(');
		if(index != string::npos){

			res = res.substr(0, index);
		}
	}
	fun->setText("Summary Unit: " +
			QString::fromStdString(res));
	this->sums = sums;
	this->function_addresses = function_addresses;


	this->su = su;
	run->setEnabled(true);
}

void MemoryVisualizer::clear()
{
	fun->setText("No Summary Unit selected.");
	this->sums = NULL;
	dotties.clear();
	run->setEnabled(false);
	statements->clear();
	drawer->clear();
	statements->setColumnCount(1);
	QStringList l;
	l.push_back("Statement");
	statements->setHorizontalHeaderLabels(l);
	//stmt_tree->setRootIsDecorated(false);
	this->su = NULL;
	this->function_addresses = NULL;
	this->node = NULL;
	this->cg = NULL;
	to_display.clear();
	selected = NULL;

}

MemoryVisualizer::~MemoryVisualizer()
{

}
