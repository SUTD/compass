/*
 * SummaryUnit.cpp
 *
 *  Created on: Mar 6, 2009
 *      Author: isil
 */

#include "SummaryUnit.h"
#include "Variable.h"
#include "SuperBlock.h"
#include "Function.h"

#include "location.h"
#include "node.h"
#include "variable_declaration.h"

namespace sail{

SummaryUnit::SummaryUnit()
{


}

bool SummaryUnit::is_local(Variable* v)
{

	cout << "Checking if " << v->to_string() << " is local in " <<
			this->get_identifier().to_string() << endl;
	if(is_function()) return v->is_local();
	il::variable_declaration* vd = v->get_declaration();
	if(vd == NULL && get_identifier().get_language() != LANG_JAVA) {
		cout << "Var decl is null!" << endl;
		return true;
	}

	return false;

	/*
	 * THIS DOES NOT WORK FOR C++!
	 */
	il::location l = vd->get_location();
	SuperBlock* sb = (SuperBlock*) this;

	Instruction* first = sb->get_first_instruction();
	Instruction* last = sb->get_last_instruction();

	if(last == NULL){
		bool res = ((int)l.line_start>=(int)first->line);
		return res;
	}
	bool res = ((int)l.line_start>=(int)first->line &&
			(int)l.line_start<=(int)last->line);


	return res;
}



SummaryUnit::~SummaryUnit()
{

}
}
