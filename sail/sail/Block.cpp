/*
 * Block.cpp
 *
 *  Created on: Jul 11, 2008
 *      Author: isil
 */

#include "Block.h"
#include "CfgEdge.h"

namespace sail {

Block::Block() {
block_id = -99;

}

int Block::get_block_id() const
{
	return block_id;
}

void Block::set_block_id(int id)
{
	this->block_id = id;
}

set<CfgEdge*>& Block::get_successors()
{
	return successors;
}
set<CfgEdge*>& Block::get_predecessors()
{
	return predecessors;
}

void Block::add_successor_edge(CfgEdge* edge)
{
	successors.insert(edge);
}
void Block::add_predecessor_edge(CfgEdge* edge)
{
	predecessors.insert(edge);
}

bool Block::is_basicblock()
{
	return false;
}

bool Block::is_superblock()
{
	return false;
}

//haiyan added
map<Symbol*, CfgEdge*>& Block::get_successors_map(){
	map<Symbol*, CfgEdge*>* s_map = new map<Symbol*, CfgEdge*>();
	set<CfgEdge*> succs = this->get_successors();
	for(set<CfgEdge*>::iterator it = succs.begin(); it != succs.end(); it++){
		Symbol* sym = (*it)->get_cond();
		(*s_map)[sym] = *it;
	}
	return *s_map;
}
//haiyan ended

Block::~Block() {

}

}
