#include "statement.h"
#include "assert.h"

namespace il
{

statement::statement()
{
}

void statement::assert_statement()
{
	assert(this->node_type_id == ASSEMBLY ||
	       this->node_type_id == BLOCK ||
	       this->node_type_id == BREAK_STMT ||
	       this->node_type_id == CASE_LABEL ||
	       this->node_type_id == CONTINUE_STMT ||
	       this->node_type_id == DO_WHILE_LOOP ||
	       this->node_type_id == EXPR_INSTRUCTION ||
	       this->node_type_id == FOR_LOOP ||
	       this->node_type_id == GOTO_STMT ||
	       this->node_type_id == IF_STMT ||
	       this->node_type_id == LABEL ||
	       this->node_type_id == NOOP_INSTRUCTION ||
	       this->node_type_id == RETURN_STMT ||
	       this->node_type_id == SET_INSTRUCTION ||
	       this->node_type_id == SWITCH_STMT ||
	       this->node_type_id == TRY_CATCH_STMT ||
	       this->node_type_id == TRY_FINALLY_STMT ||
	       this->node_type_id == WHILE_LOOP);
}

statement::~statement()
{
}

}
