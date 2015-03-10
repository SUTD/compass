/*
 * GlobalAnalysisState.cpp
 *
 *  Created on: Apr 9, 2009
 *      Author: tdillig
 */

#include "GlobalAnalysisState.h"
#include <iostream>
#include "Callgraph.h"
#include "DataManager.h"
#include "sail/Function.h"
#include "sail/BasicBlock.h"
#include "sail/Cfg.h"
#include "sail/Instruction.h"
#include "sail/FunctionCall.h"

check_type GlobalAnalysisState::checks;
check_type GlobalAnalysisState::disabled_checks;



void GlobalAnalysisState::clear()
{
	checks = CHECK_NONE;
	disabled_checks = CHECK_NONE;
}


void GlobalAnalysisState::add_check_type(check_type ct)
{
	checks = check_type(checks | ct);
}
bool GlobalAnalysisState::check_buffer()
{
	return checks & CHECK_BUFFER;
}
bool GlobalAnalysisState::check_null()
{
	return checks & CHECK_NULL;
}
bool GlobalAnalysisState::check_uninit()
{
	return checks & CHECK_UNINIT;
}

bool GlobalAnalysisState::check_casts()
{
	return checks & CHECK_CAST;
}

bool GlobalAnalysisState::check_memory_leaks()
{
	return checks & CHECK_MEMORY_LEAK;
}

bool GlobalAnalysisState::check_double_deletes()
{
	return (checks & CHECK_DOUBLE_DELETE || disabled_checks & CHECK_DOUBLE_DELETE);
}

bool  GlobalAnalysisState::check_deleted_access()
{
	return (checks & CHECK_DELETED_ACCESS);
}

bool GlobalAnalysisState::track_delete()
{
	if ((checks & CHECK_MEMORY_LEAK) ||  (checks & CHECK_DOUBLE_DELETE) ||
			(checks & CHECK_DELETED_ACCESS))
		return true;



	return ((disabled_checks & CHECK_MEMORY_LEAK) ||
			(disabled_checks & CHECK_DOUBLE_DELETE) ||
			(disabled_checks & CHECK_DELETED_ACCESS));
}

void GlobalAnalysisState::disable_checks()
{
	disabled_checks = checks;
	checks = CHECK_NONE;
}

void GlobalAnalysisState::enable_checks()
{
	checks = disabled_checks;
	disabled_checks = CHECK_NONE;
}

void GlobalAnalysisState::enable_buffer_check()
{
	GlobalAnalysisState::add_check_type(CHECK_BUFFER);
}
void GlobalAnalysisState::enable_null_check()
{
	GlobalAnalysisState::add_check_type(CHECK_NULL);
}
void GlobalAnalysisState::enable_uninit_check()
{
	GlobalAnalysisState::add_check_type(CHECK_UNINIT);
}

void GlobalAnalysisState::enable_cast_check()
{
	GlobalAnalysisState::add_check_type(CHECK_CAST);
}

void GlobalAnalysisState::enable_memory_leak_check()
{
	GlobalAnalysisState::add_check_type(CHECK_MEMORY_LEAK);
}

void GlobalAnalysisState::enable_double_delete_check()
{
	GlobalAnalysisState::add_check_type(CHECK_DOUBLE_DELETE);
}

void GlobalAnalysisState::enable_deleted_access_check()
{
	GlobalAnalysisState::add_check_type(CHECK_DELETED_ACCESS);
}


void GlobalAnalysisState::set_analysis_options(CGNode* check_fn,
		DataManager & dm)
{
	if(check_fn == NULL) return;
	sail::Function* f = (sail::Function*) dm.get_data(check_fn->get_serial_id(),
			SAIL_FUNCTION);
	sail::BasicBlock* b = f->get_cfg()->get_entry_block();
	vector<sail::Instruction*> & insts = b->get_statements();
	for(unsigned int i=0; i < insts.size(); i++)
	{
		sail::Instruction* inst = insts[i];
		if(inst->get_instruction_id()!= sail::FUNCTION_CALL) continue;
		sail::FunctionCall* fc = (sail::FunctionCall*) inst;
		if(fc->get_function_name() == "check_null")
		{
			GlobalAnalysisState::add_check_type(CHECK_NULL);
		}
		else if(fc->get_function_name() == "check_buffer")
		{
			GlobalAnalysisState::add_check_type(CHECK_BUFFER);
		}
		else if(fc->get_function_name() == "check_uninit" ||
				fc->get_function_name() == "check_uninitialized")
		{
			GlobalAnalysisState::add_check_type(CHECK_UNINIT);
		}

		else if(fc->get_function_name() == "check_cast" ||
				fc->get_function_name() == "check_casts")
		{
			GlobalAnalysisState::add_check_type(CHECK_CAST);
		}

		else if(fc->get_function_name() == "check_memory_leak")
		{
			GlobalAnalysisState::add_check_type(CHECK_MEMORY_LEAK);
		}


		else if(fc->get_function_name() == "check_double_delete")
		{
			GlobalAnalysisState::add_check_type(CHECK_DOUBLE_DELETE);
		}

		else if(fc->get_function_name() == "check_deleted_access")
		{
			GlobalAnalysisState::add_check_type(CHECK_DELETED_ACCESS);
		}

	}
}

string GlobalAnalysisState::get_analysis_options()
{
	string res = "Options: ";
	if(checks == CHECK_NONE){
		res+= "none";
		return res;
	}
	if(check_null()) res+="null ";
	if(check_buffer()) res+="buffer ";
	if(check_uninit()) res+="uninit ";
	if(check_casts()) res += "cast ";
	return res;
}
