/*
 * compass-serialization.cpp
 *
 *  Created on: Nov 23, 2009
 *      Author: tdillig
 */

#define IN_COMPASS_SERIALIZATION true

#include "sail-serialization.h"
#include "compass-serialization.h"
#include "serialization-hooks.h"

#include "boost-serialization-decls.h"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "Function.h"
#include "Serializable.h"


Serializable* load_serializable(ifstream & ss, bool sail_body)
{
	if(sail_body)
	{
		begin_serialization();
		sail::Function* s = NULL;
		IN_ARCHIEVE ia(ss);

		ia >> s;
		//cout << "SS is: " << ss.rdbuf() << endl;
		end_serialization();
		return s;

	}
	SummaryStream* s;
	IN_ARCHIEVE ia(ss);

	ia >> s;
	//cout << "SS is: " << ss.rdbuf() << endl;
	end_serialization();
	return s;


/*
	begin_serialization();
	Serializable* s = NULL;
	IN_ARCHIEVE ia(ss);

	ia >> s;
	//cout << "SS is: " << ss.rdbuf() << endl;
	end_serialization();
	sail::Function* f = (sail::Function*) s;
	cout << "s: " << s << endl;
	cout << "f: " << f << endl;
	cout << "load serializable fn print MO MO MO MO " << f->to_string()
	<< endl;
	cout << "Finished load... " << endl;
	return s;*/

}


void write_serializable(ofstream& ss, Serializable* s, bool sail_fn)
{
	if(sail_fn)
	{
		sail::Function* f = (sail::Function*)s;
		OUT_ARCHIEVE oa(ss);
		oa << f;
		return;

	}
	SummaryStream* sum_s = (SummaryStream*)s;
	OUT_ARCHIEVE oa(ss);
	oa << sum_s;

}



SummaryGraph* load_summary_graph(stringstream& ss)
{
	begin_serialization();
	SummaryGraph* new_sg;
	IN_ARCHIEVE ia(ss);
	ia >> new_sg;
	end_serialization();
	return new_sg;

}

void write_summary_graph(stringstream & ss, SummaryGraph* sg)
{

	OUT_ARCHIEVE oa(ss);
	oa << sg;
}

AnalysisResult* load_analysis_result(stringstream & ss)
{
	begin_serialization();
	AnalysisResult* new_ar;
	IN_ARCHIEVE ia(ss);
	ia >> new_ar;
	end_serialization();
	return new_ar;

}

void write_analysis_results(stringstream & ss, AnalysisResult* ar)
{
	OUT_ARCHIEVE oa(ss);
	oa << ar;
}

dotty_info* load_dotty_info(stringstream& ss)
{
	begin_serialization();
	dotty_info* new_info;
	IN_ARCHIEVE ia(ss);
	ia >> new_info;
	end_serialization();
	return new_info;
}

void write_dotty_info(stringstream& ss, dotty_info* di)
{
	OUT_ARCHIEVE oa(ss);
	oa << di;
}

ProjectSettings* load_project_settings(ifstream& ss)
{
	begin_serialization();
	ProjectSettings* new_ps;
	IN_ARCHIEVE ia(ss);
	ia >> new_ps;
	end_serialization();
	return new_ps;
}
void write_project_settings(stringstream& ss, ProjectSettings* ps)
{
	OUT_ARCHIEVE oa(ss);
	oa << ps;
}
