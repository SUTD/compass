/*
 * ClientAnalysisManager.h
 *
 *  Created on: Sep 1, 2012
 *      Author: tdillig
 */

#ifndef CLIENTANALYSISMANAGER_H_
#define CLIENTANALYSISMANAGER_H_
#include <vector>
#include <map>
#include "constraint.h"

#define CLIENT_INITIAL_OFFSET -16

namespace sail
{
class Instruction;
class Symbol;
class FunctionCall;
}
using namespace std;

class ClientAnalysis;
class MemoryAnalysis;
class Callgraph;

class ClientAnalysisManager {
private:
	int next_offset;
	vector<ClientAnalysis*> analyses;
	map<ClientAnalysis*, int> offsets;
	map<int, ClientAnalysis*> rev_offsets;

	MemoryAnalysis* ma;

	int var_counter;
public:
	ClientAnalysisManager();
	const map<int, ClientAnalysis*> & get_client_analyses();
	void register_memory_analysis(MemoryAnalysis* ma);
	void register_client(ClientAnalysis* a);
	ClientAnalysis* get_analyis_from_offset(int offset);
	void clear();
	~ClientAnalysisManager();

	void process_function_call_before(sail::FunctionCall* inst, bool known);
	void process_function_call_after(sail::FunctionCall* inst, bool known);

	void process_instruction_before(sail::Instruction* inst);
	void process_instruction_after(sail::Instruction* inst);

	void process_inst_before(sail::Instruction* inst, ClientAnalysis* a);
	void process_inst_after(sail::Instruction* inst, ClientAnalysis* a);

	void write_state(ClientAnalysis* a, sail::Variable* lhs, sail::Symbol* rhs,
			Constraint c);
	sail::Variable* read_state(ClientAnalysis* a, sail::Symbol* s);
	Constraint get_neqz_constraint(sail::Symbol* s);

	void assert_state(Constraint c, const string & msg);

	sail::Variable* get_temp(il::type* t);

	sail::Variable* load(sail::Variable* v, int offset);
	sail::Variable* load(sail::Variable* v, sail::Symbol* index);

	sail::Variable* get_unknown_value(il::type* t, const string & name);
	bool report_error(const string & msg, Constraint fail_c);



};

#endif /* CLIENTANALYSISMANAGER_H_ */
