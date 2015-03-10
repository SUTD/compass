/*
 * POEKey.cpp
 *
 *  Created on: Jun 20, 2012
 *      Author: boyang
 */

#include "POEKey.h"



bool ComparePOEKey::operator ()(const POEKey* k1, const POEKey* k2) const
{
	if(k1->get_context_size() < k2->get_context_size())
	{
		return true;
	}
	if(k1->get_context_size() > k2->get_context_size())
	{
		return false;
	}

	if(k1->get_block_id()== -1 && k2->get_block_id()==-1){
		if(k1-k2 < 0) return true;
		return false;
	}

	return (k1->get_block_id() < k2->get_block_id());
}



POEKey::POEKey() {
	// TODO Auto-generated constructor stub

}

POEKey::~POEKey() {
	// TODO Auto-generated destructor stub
}


POEKey::POEKey(Block *block, CallingContext *callingcontext)
{
	this->block = block;
	this->callingcontext = callingcontext;
}


bool POEKey::is_basicblock()
{
	return this->block->is_basicblock();
}


bool POEKey::is_superblock()
{
	return this->block->is_superblock();
}


Block * POEKey::getBlock()
{
	return this->block;
}


set<CfgEdge*> POEKey::get_successors()
{
	return this->block->get_successors();
}


set<CfgEdge*> POEKey::get_predecessors()
{
	return this->block->get_predecessors();
}


CallingContext * POEKey::getcallingcontext()
{
	return this->callingcontext;
}


int POEKey::get_context_size() const
{
	return callingcontext->get_context_size();
}


int POEKey::get_block_id() const
{
	return block->get_block_id();
}


bool POEKey::operator==(const POEKey &  other)const
{
	if((*callingcontext) == (*other.callingcontext) && this->block->get_block_id() == other.block->get_block_id() &&this->block->is_superblock() == other.block->is_superblock())
	{
		return true;
	}
	else
	{
		return false;
	}
}


string POEKey::to_string() const
{
	string res =  "\n---------------\nb-ID :\t";
	res += std::to_string(block->get_block_id()) + "\n";
	if(block->is_superblock())
	{
		res+= "is SuperBlock \n";
	}
	res += callingcontext->to_string();
	res += "---------------" ;
	return res;
}
