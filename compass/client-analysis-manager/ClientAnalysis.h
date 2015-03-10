/*
 * ClientAnalysis.h
 *
 *  Created on: Aug 31, 2012
 *      Author: tdillig
 */

#ifndef CLIENTANALYSIS_H_
#define CLIENTANALYSIS_H_

#include <string>
#include "sail.h"
#include "type.h"
#include <map>
#include "ClientAnalysisManager.h"
#include "constraint.h"

class ClientAnalysis;

using namespace std;

class ClientAnalysis {
	friend class MemoryAnalysis;
	friend class MemoryGraph;
private:
	static ClientAnalysisManager cam;

public:

	static void clear_clients();
	static const map<int, ClientAnalysis*> & get_client_analyses();
	static void register_client(ClientAnalysis* a);

	static bool report_error(const string & msg, Constraint fail_c);

	ClientAnalysis();

	/*
	 * The name of the property client wants to track.
	 */
	virtual string get_property_name() = 0;

	/*
	 * The type of the property being tracked
	 */
	virtual il::type* get_property_type() = 0;

	/*
	 * Initial value for the tracked property, null
	 * if not applicable.
	 */
	virtual sail::Symbol* get_initial_value() = 0;


	/*
	 * For which types should this property be tracked?
	 */
	virtual bool is_tracked_type(il::type* t) =0;


	/*
	 * Transfer functions for each instruction
	 * Override this function if your transformer for this
	 * instruction is not a no-op.
	 * The function named _before gets invoked before the instruction is
	 * analyzed by the memory analysis, and the version named _after
	 * gets invoked after the instruction is analyzed the memory analysis.
	 */
	virtual void process_assign_before(sail::Assignment* inst);
	virtual void process_field_read_before(sail::FieldRefRead* inst);
	virtual void process_field_write_before(sail::FieldRefWrite* inst);
	virtual void process_load_before(sail::Load* inst);
	virtual void process_store_before(sail::Store* inst);
	virtual void process_address_var_before(sail::AddressVar* inst);
	virtual void process_unop_before(sail::Unop* inst);
	virtual void process_binop_before(sail::Binop* inst);
	virtual void process_cast_before(sail::Cast* inst);
	virtual void process_array_read_before(sail::ArrayRefRead* inst);
	virtual void process_array_write_before(sail::ArrayRefWrite* inst);
	virtual void process_address_string_before(sail::AddressString* inst);
	virtual void process_assembly_before(sail::Assembly* inst);
	virtual void process_function_call_before(sail::FunctionCall* inst,
			bool known);
	virtual void process_fn_ptr_call_before(sail::FunctionPointerCall* inst);
	virtual void process_address_label_before(sail::AddressLabel* inst);


	virtual void process_assign_after(sail::Assignment* inst);
	virtual void process_field_read_after(sail::FieldRefRead* inst);
	virtual void process_field_write_after(sail::FieldRefWrite* inst);
	virtual void process_load_after(sail::Load* inst);
	virtual void process_store_after(sail::Store* inst);
	virtual void process_address_var_after(sail::AddressVar* inst);
	virtual void process_unop_after(sail::Unop* inst);
	virtual void process_binop_after(sail::Binop* inst);
	virtual void process_cast_after(sail::Cast* inst);
	virtual void process_array_read_after(sail::ArrayRefRead* inst);
	virtual void process_array_write_after(sail::ArrayRefWrite* inst);
	virtual void process_address_string_after(sail::AddressString* inst);
	virtual void process_assembly_after(sail::Assembly* inst);
	virtual void process_function_call_after(sail::FunctionCall* inst,
			bool known);
	virtual void process_fn_ptr_call_after(sail::FunctionPointerCall* inst);
	virtual void process_address_label_after(sail::AddressLabel* inst);
	virtual bool process_error(const string & msg, Constraint fail_c);


	sail::Variable* get_temp(il::type* t);

	//------------------------------------------------------------------------
	// FUNCTIONS CALLED BY CLIENT ANALYSIS TO MANIPULATE/QUERY THEIR STATE
	//------------------------------------------------------------------------



	/*
	 * Change the relevant state of lhs tracked by client analysis to rhs
	 * under constraint c.
	 */
	void write_state(sail::Variable* lhs, sail::Symbol* rhs, Constraint c);

	/*
	 * Reads the relevant state of s tracked by client analysis  and
	 * writes this value to the returned variable.
	 */
	sail::Variable* read_state(sail::Symbol* s);


	//-----------------------------------------------------------------

	/*
	 * Assert something about the state tracked by the analysis
	 */
	void assert_state(Constraint c, const string & msg);

	/*
	 * Returns the constraint under which the value of s is not equal to zero.
	 */
	Constraint get_neqz_constraint(sail::Symbol* s);

	//------------------------------------------------------------------




	/*
	 * lhs->field = rhs under constraint c
	 */
	void store(sail::Symbol* lhs, sail::Symbol* rhs,
			const string & field, Constraint c);

	/*
	 * (*lhs)[i] = rhs under constraint c
	 */
	void store(sail::Symbol* lhs, sail::Symbol* rhs,
			sail::Symbol* index, Constraint c);


	/*
	 * ret = s->field
	 */
	sail::Variable* load(sail::Variable* s, int offset);

	/*
	 * ret = *s[index]
	 */
	sail::Variable* load(sail::Variable* v,
			sail::Symbol* index);


	sail::Variable* get_unknown_value(il::type* t, const string & name);




	virtual ~ClientAnalysis();
};

#endif /* CLIENTANALYSIS_H_ */
