/*
 * CfgEdge.cpp
 *
 *  Created on: Jul 6, 2008
 *      Author: isil
 */

#include "CfgEdge.h"
#include <assert.h>
#include <vector>
#include "util.h"
#include "BasicBlock.h"
#include "Symbol.h"
#include "Block.h"

namespace sail {

CfgEdge::CfgEdge(Block* source, Block* target, Symbol* cond) {
	assert(source!=NULL);
	this->source = source;
	this->target = target;
	this->cond = cond;
	this->backedge = false;

}

CfgEdge::CfgEdge(const CfgEdge& other)
{
	this->source = other.source;
	this->target = other.target;
	this->cond = other.cond;
	this->backedge = false;

}

Block* CfgEdge::get_source()
{
	return source;
}
Block* CfgEdge::get_target()
{
	return target;
}

bool CfgEdge::is_backedge()
{
	return backedge;
}

void CfgEdge::mark_backedge()
{
	this->backedge = true;
}

void CfgEdge::unmark_backedge()
{
	this->backedge = false;
}


void CfgEdge::set_target(Block* b)
{
	this->target = b;
}

void CfgEdge::set_source(Block* b)
{
	this->source = b;
}

void CfgEdge::set_cond(Symbol* cond)
{
	this->cond = cond;
}

/*
 * Can return NULL to indicate an uncoditional edge.
 */
Symbol* CfgEdge::get_cond()
{
	return cond;
}
string CfgEdge::to_string() const
{
	string res = int_to_string(source->get_block_id());
	res += "->";
	res+= target? int_to_string(target->get_block_id()) : "NULL";
	res += " / ";
	res += cond ? cond->to_string() : "true";
	return res;
}

string CfgEdge::to_string(bool pretty_print) const
{
	string res = int_to_string(source->get_block_id());
	res += "->" + int_to_string(target->get_block_id()) + " / ";
	res += cond ? cond->to_string(pretty_print) : "true";
	return res;
}



CfgEdge::~CfgEdge() {

}

}
