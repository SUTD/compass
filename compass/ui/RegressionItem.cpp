/*
 * RegressionItem.cpp
 *
 *  Created on: Jan 4, 2010
 *      Author: tdillig
 */

#include "RegressionItem.h"

#include "CNode.h"
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
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include "Regressions.h"
#include "SailGenerator.h"

#include <dirent.h>
#include "fileops.h"
#include <errno.h>
#include "RegressionRunner.h"


#include <sys/stat.h>
#include <sys/types.h>


/*
 * Maximum memory (in MB) used by one regression.
 */
#define MAX_REGRESS_MEMORY 4000


#define SUCCESS 1
#define ERROR 0
#define CRASH -1


RegressionItem::RegressionItem(string folder, set<Error> & expected_output,
		RegressionRunner* rr, Regressions* r, QSemaphore & s, int c):folder(folder),
		expected_output(expected_output), s(s), c(c)
{
	QObject::connect(this,SIGNAL(unit_finished(QString, double, bool ,
					bool, int)),r,SLOT(regression_finished(QString, double, bool ,
							bool, int)));
	this->rr =rr;
}






void close_pipe(int * p)
{
	close(p[0]);
	close(p[1]);

}



bool RegressionItem::run_regression_in_given_folder(string folder,
		set<Error> & expected_output, double & time, bool & crashed,
		int & lines)
{


	int p[2];

	int pipe_status = pipe(p);
	assert(pipe_status == 0);


	//set<string> files;
	//collect_all_c_files(files, folder + "/");

	string regress_dir = "/tmp/regress_" + int_to_string(c);
	remove_directory(regress_dir);
	mkdir(regress_dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) ;

    string sail_summary_dir = regress_dir + "/sail";
    string compass_summary_dir= regress_dir + "/compass";



	rr->cancel_mutex.lock();
	if(rr->canceled){
		remove_directory(regress_dir);
		rr->cancel_mutex.unlock();
		close_pipe(p);
		return true;
	}
	int p_id = fork();
	if(p_id != 0){
		rr->p_ids.insert(p_id);
	}
	if(p_id == 0) setpgrp();
	if(p_id != 0) rr->cancel_mutex.unlock();



	if(p_id == 0)
	{

		GlobalAnalysisState::clear();
		AccessPath::clear();
		Constraint::clear();
		il::type::clear();
		// New process
		/*
		 * First, seed the pipe in case we crash
		 */
		int v = ERROR;
		float f = 0.0;
		//status
		int s = write(p[1], &v, sizeof(int));
		assert(s > 0);

		//time (s)
		s = write(p[1], &f, sizeof(float));
		assert(s > 0);

		//total lines
		s = write(p[1], &v, sizeof(int));
		assert(s > 0);

		//change our working directory to the current regression folder
		s = chdir(regress_dir.c_str());
		assert(s == 0);

		/*
		 * Compile all the files
		 */
		set<Error*> compilation_errors;
		SailGenerator sg(folder, sail_summary_dir, compilation_errors, true);
		int num_lines = sg.get_total_lines();

		if(compilation_errors.size() > 0)
		{
			/*
			 * Default seed in pipe is appropriate.
			 */
			_exit(0);
		}

		/*
		 * Now, run the analysis.
		 */
		GlobalAnalysisState::clear();

		map<sum_data_type, string> paths;
		paths[SAIL_FUNCTION] = sail_summary_dir;
		paths[COMPASS_SUMMARY] = compass_summary_dir;

		DataManager dm(MAX_REGRESS_MEMORY, paths);


		set<string> fn_ids;
		dm.get_serializable_ids(fn_ids, SAIL_FUNCTION);


		map<string, set<Error*> > errors;
		Callgraph cg(fn_ids, &dm, errors);
		GlobalAnalysisState::set_analysis_options(cg.get_checks_fn(), dm);
		MemoryAnalysis ma;
		Manager m(&cg, false, &ma, &dm, 1, 60, Identifier(), NULL);

		/*
		 * We did not crash, so empty out pipe of dummy values.
		 */
		int t = 0;
		float tt = 0.0;
		s = read(p[0], &t, sizeof(int));
		s = read(p[0], &tt, sizeof(float));
		s = read(p[0], &t, sizeof(int));
		t = SUCCESS;


		vector<Error*>  out = m.get_errors();
		{
			map<string, set<Error*> >::iterator it = errors.begin();
			for(; it != errors.end(); it++)
			{
				set<Error*>::iterator it2 = it->second.begin();
				for(; it2 != it->second.end(); it2++)
					out.push_back(*it2);
			}
		}


		bool crash = false;
		float time = 0.0;

		/*
		 * Add up total cpu time
		 */
		map<Identifier, AnalysisResult*>::const_iterator ai =
				m.get_results().begin();
		for(; ai != m.get_results().end(); ai++)
		{
			AnalysisResult* ar = ai->second;
			time += ar->time;
		}



		bool success = true;
		set<Error> actual_output;
		for(unsigned int i=0; i < out.size(); i++) {
			Error* cur_error = out[i];
			actual_output.insert(*cur_error);
		}


		if(crash || actual_output.size() != expected_output.size()){
			success = false;
		}
		else
		{
			set<Error>::iterator it = actual_output.begin();
			for(; it != actual_output.end(); it++)
			{
				Error cur_error = *it;

				 // We need to normalize the filename;

				string file = cur_error.file;
				cout << "file: " << file << endl;
				cout << "folder: " << folder << endl;

				//for dex, the path will be the folder of the output artifact
				if(file.find("/tmp/regress_")!= string::npos) {
					size_t end = file.substr(13).find('/');
					//cout << "end: " << end;
					assert(end != string::npos);
					file = file.substr(13+end+1);
					//cout << "file: " << file << endl;
				}
				else
				{
					size_t pos = file.rfind(folder);
					assert(pos!=string::npos);
					file = file.substr(pos + folder.size() + 1);
				}
				cur_error.file = file;
				if(expected_output.count(cur_error) == 0){

					success = false;
					break;
				}
			}


		}
		int ss = 0;
		if(success) ss = 1;
		if(crash) ss = -1;



		s = write(p[1], &ss, sizeof(int));
		s = write(p[1], &time, sizeof(float));
		s = write(p[1], &num_lines, sizeof(int));
		_exit(0);
	}

	//Old process
	int child_exit_status;
	waitpid( p_id, &child_exit_status, 0);

	rr->cancel_mutex.lock();
	rr->p_ids.erase(p_id);
	rr->cancel_mutex.unlock();

	//clean up directory
	remove_directory(regress_dir);

	bool success = true;
	float regress_time = 0;
	int ss = 0;
	int num_lines = 0;

	int ignore = read(p[0], &ss, sizeof(int));
	success = (ss == 1);
	crashed = (ss == -1);
	if(child_exit_status != 0) {
		success = false;
		crashed = true;
		regress_time = -1;
	}

	ignore = read(p[0], &regress_time, sizeof(float));
	time = regress_time;

	ignore = read(p[0], &num_lines, sizeof(int));
	lines = num_lines;

	close_pipe(p);
	return success;
}


void RegressionItem::run()
{
	double time = 0.0;
	bool crashed = false;
	int num_lines = 0;
	bool success = run_regression_in_given_folder(folder, expected_output, time,
			crashed, num_lines);


	QString id = QString::fromStdString(folder);
	{
		rr->cancel_mutex.lock();
		if(rr->canceled) {
			s.release();
			rr->cancel_mutex.unlock();

			return;
		}
		rr->cancel_mutex.unlock();
	}

	emit unit_finished(id, time, success, crashed, num_lines);
	s.release();
}
